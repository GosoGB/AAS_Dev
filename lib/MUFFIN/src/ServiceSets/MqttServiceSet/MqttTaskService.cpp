/**
 * @file MqttTaskService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 태스크를 실행하고 정지하는 서비스를 정의합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <Preferences.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "DataFormat/JSON/JSON.h"


#include "Core/Core.h"
#include "Core/Task/ModbusTask.h"
#include "Core/Task/MelsecTask.h"
#include "Protocol/Modbus/ModbusMutex.h"
#include "Protocol/Melsec/MelsecMutex.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Melsec/Melsec.h"
#include "Protocol/Modbus/Include/ArduinoModbus/src/ModbusRTUClient.h"

#include "JARVIS/JARVIS.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "JARVIS/Config/Network/CatM1.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "Protocol/SPEAR/SPEAR.h"
#include "ServiceSets/FirmwareUpdateServiceSet/ParseUpdateInfoService.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"
#include "ServiceSets/MqttServiceSet/MqttTaskService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/Node/Include/Utility.h"



static TaskHandle_t xHandle = NULL;
static uint8_t RECONNECT_TRIAL_COUNT = 0;
muffin::CallbackUpdateInitConfig cbUpdateInitConfig = nullptr;


namespace muffin {


    Status manageConnection()
    {
        if (mqttClient == nullptr)
        {
            LOG_ERROR(logger, "MQTT CLIENT DOES NOT EXIST");
            return Status(Status::Code::BAD_NOT_EXECUTABLE);
        }

        if (mqttClient->IsConnected() == Status::Code::GOOD)
        {
            return Status(Status::Code::GOOD);
        }
        
        INetwork* snic = RetrieveServiceNicService();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }
        
        Status ret = mqttClient->Disconnect(mutex.second);
        if (ret != Status::Code::GOOD)
        {
            snic->ReleaseMutex();
            goto ON_FAIL;
        }
        snic->ReleaseMutex();
        ret = InitMqttClientService();
        if (ret != Status::Code::GOOD)
        {
            goto ON_FAIL;
        }

        ret = ConnectMqttClientService();
        if (ret != Status::Code::GOOD)
        {
            goto ON_FAIL;
        }

        LOG_INFO(logger, "Reconnected to the MQTT broker");
        snic->ReleaseMutex();
        RECONNECT_TRIAL_COUNT = 0;
        return ret;
        
    ON_FAIL:
        if (++RECONNECT_TRIAL_COUNT == MAX_RETRY_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO RECONNECT TO THE BROKER");
            LOG_INFO(logger, "Restart the device");
        #if defined(MT10) || defined(MB10)
            spear.Reset();
        #endif 
            esp_restart();
        }

        snic->ReleaseMutex();
        return ret;
    }

    Status publishMessages()
    {
        if (mqtt::cdo.Count() == 0)
        {
            return Status(Status::Code::GOOD);
        }

        INetwork* snic = RetrieveServiceNicService();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }
        
        Status ret(Status::Code::UNCERTAIN);
        while (mqtt::cdo.Count() > 0)
        {
            uint8_t trialCount = 0;

            const std::pair<Status, mqtt::Message> message = mqtt::cdo.Peek();
            if (message.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO PEEK MESSAGE: %s ", message.first.c_str());
                continue;
            }

            for (; trialCount < MAX_RETRY_COUNT; ++trialCount)
            {
                ret = mqttClient->Publish(mutex.second, message.second);
                if (ret == Status::Code::GOOD)
                {
                    mqtt::cdo.Retrieve();
                    break;
                }
            }

            if (trialCount == MAX_RETRY_COUNT)
            {
                LOG_WARNING(logger, "FAILED TO PUBLISH MESSAGE: %s", ret.c_str());
                mqtt::cdo.Retrieve();
                break;
            }
        }
        
        snic->ReleaseMutex();
        return ret;
    }

    Status PublishResponseJARVIS(const jarvis_struct_t& response)
    {
        JSON json;
        Status ret(Status::Code::UNCERTAIN);
        const std::string payload = json.Serialize(response);
        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, std::move(payload));

        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            ret = mqtt::cdo.Store(message);
            if (ret == Status::Code::GOOD)
            {
                return ret;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        LOG_ERROR(logger, "FAIL TO STORE JARVIS RESPONSE: %s", ret.c_str());
        return ret;
    }

    Status processMessageJARVIS(init_cfg_t& params, const char* payload)
    {
        jarvis_struct_t response;
        response.SourceTimestamp = GetTimestampInMillis();

        if (params.HasPendingJARVIS == true || params.HasPendingUpdate == true)
        {
            response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
            response.Description = "UNAVAILABLE DUE TO JARVIS TASK BEING ALREADY RUNNING OR BLOCKED";
            return PublishResponseJARVIS(response);
        }
        
        JSON json;
        JsonDocument doc;

        Status ret = json.Deserialize(payload, &doc);
        if (ret != Status::Code::GOOD)
        {
            switch (ret.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_COMMUNICATION);
                response.Description   = "PAYLOAD INSUFFICIENT OR INCOMPLETE";
                break;
            case Status::Code::BAD_NO_DATA:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE);
                response.Description   = "PAYLOAD EMPTY";
                break;
            case Status::Code::BAD_DATA_ENCODING_INVALID:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_DECODING_ERROR);
                response.Description   = "PAYLOAD INVALID ENCODING";
                break;
            case Status::Code::BAD_OUT_OF_MEMORY:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_OUT_OF_MEMORY);
                response.Description   = "PAYLOAD OUT OF MEMORY";
                break;
            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_DECODING_CAPACITY_EXCEEDED);
                response.Description   = "PAYLOAD EXCEEDED NESTING LIMIT";
                break;
            case Status::Code::BAD_UNEXPECTED_ERROR:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
                response.Description   = "UNDEFINED CONDITION";
                break;
            default:
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
                response.Description   = "UNDEFINED CONDITION";
                break;
            }
            return PublishResponseJARVIS(response);
        }
        
        const auto version = Convert.ToJarvisVersion(doc["ver"].as<const char*>());
        if ((version.first.ToCode() != Status::Code::GOOD) || (version.second != jvs::prtcl_ver_e::VERSEOIN_4))
        {
            response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_INVALID_VERSION);
            response.Description   = "INVALID OR UNSUPPORTED PROTOCOL VERSION";
            return PublishResponseJARVIS(response);
        }

        if (doc.containsKey("rqi") == true)
        {
            const char* rqi = doc["rqi"].as<const char*>();
            if ((rqi == nullptr) || (strlen(rqi) == 0))
            {
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD);
                response.Description   = "INVALID REQUEST ID: CANNOT BE NULL OR EMPTY";
                return PublishResponseJARVIS(response);
            }
        }

        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            INetwork* snic = RetrieveServiceNicService();
            std::pair<Status, size_t> mutex = snic->TakeMutex();
            if (mutex.first != Status::Code::GOOD)
            {
                vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                continue;
            }
            
            // Status ret = mqttClient->Disconnect(mutex.second);
            // if (ret != Status::Code::GOOD)
            // {
            //     vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
            //     snic->ReleaseMutex();
            //     continue;
            // }
            mqttClient->Disconnect(mutex.second);

            params.HasPendingJARVIS = true;
            ASSERT((cbUpdateInitConfig != nullptr), "INIT CONFIG UPDATE CALLBACK MUST NOT BE NULL");
            ret = cbUpdateInitConfig(params);

            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger,"Device will be reset due to JARVIS request");
            #if defined(MT10) || defined(MB10)
                spear.Reset();
            #endif
                esp_restart();
            }
        }
        
        LOG_ERROR(logger, "FAILED TO UPDATE INIT CONFIG DUE TO THE SECONDARY MEMORY MALFUNCTION");
        response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_HARDWARE_FAILURE);
        response.Description   = "HARDWARE FAILURE: SECONDARY MEMORY MALFUNCTIONED";
        return PublishResponseJARVIS(response);
    }

    Status setRS485(std::vector<jvs::config::Base*>& vectorRS485CIN, jarvis_interface_struct_t* response)
    {
        response->RS485.clear();
        response->RS485.reserve(vectorRS485CIN.size());
        for (auto& rs485CIN : vectorRS485CIN)
        {
            jvs::config::Rs485* cin = static_cast<jvs::config::Rs485*>(rs485CIN);
            jarvis_rs485_struct_t rs485;
            rs485.BaudRate  = cin->GetBaudRate().second;
            rs485.DataBit   = cin->GetDataBit().second;
            rs485.ParityBit = cin->GetParityBit().second;
            rs485.PortIndex = cin->GetPortIndex().second;
            rs485.StopBit   = cin->GetStopBit().second;
            response->RS485.emplace_back(rs485);
        }

        return Status(Status::Code::GOOD);
    }

    Status setEthernet(jarvis_interface_struct_t* response)
    {
        response->Ethernet.IsEthernetSet = true;
        response->Ethernet.EnableDHCP = jvs::config::embeddedEthernet->GetDHCP().second;
        if (response->Ethernet.EnableDHCP == false)
        {
            response->Ethernet.StaticIPv4   = jvs::config::embeddedEthernet->GetStaticIPv4().second;
            response->Ethernet.Subnetmask   = jvs::config::embeddedEthernet->GetSubnetmask().second;
            response->Ethernet.Gateway      = jvs::config::embeddedEthernet->GetGateway().second;
            response->Ethernet.DNS1         = jvs::config::embeddedEthernet->GetDNS1().second;
            response->Ethernet.DNS2         = jvs::config::embeddedEthernet->GetDNS2().second;
        }
    
        return Status(Status::Code::GOOD);
    }

    Status setCatM1(jarvis_interface_struct_t* response)
    {
        response->CatM1.IsCatM1Set = true;
        response->CatM1.Model   = jvs::config::catM1->GetModel().second;
        response->CatM1.Country = jvs::config::catM1->GetCountry().second;

        return Status(Status::Code::GOOD);
    }

    Status processMessageJarvisStatus(const char* payload)
    {
        jarvis_interface_struct_t response;
        response.SourceTimestamp = GetTimestampInMillis();
        response.SNIC = jvs::config::operation.GetServerNIC().second;
        
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;

            switch (key)
            {
            case jvs::cfg_key_e::RS485:
                setRS485(pair.second,&response);
                break;
            case jvs::cfg_key_e::ETHERNET:
                setEthernet(&response);
                break;
            case jvs::cfg_key_e::LTE_CatM1:
                setCatM1(&response);
                break;
            default:
                break;
            }
        }

        Status ret = Status(Status::Code::UNCERTAIN);

        JSON json;
        std::string ResponsePayload = json.Serialize(response);
        mqtt::Message message(mqtt::topic_e::JARVIS_INTERFACE_RESPONSE, std::move(ResponsePayload));

        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            ret = mqtt::cdo.Store(message);
            if (ret == Status::Code::GOOD)
            {
                return ret;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        return ret;
    }

    Status processMessageUpdate(init_cfg_t& params, const char* payload)
    {
        if (params.HasPendingUpdate == true)
        {
            #if defined(MT10) || defined(MB10)
                spear.Reset();
            #endif
                esp_restart();
        }
        
        ota::fw_info_t* esp32 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));
        ota::fw_info_t* mega2560 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));

        Status ret = ParseUpdateInfoService(payload, esp32, mega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE: %s", ret.c_str());
            return ret;
        }

        params.HasPendingUpdate = true;
        ASSERT((cbUpdateInitConfig != nullptr), "INIT CONFIG UPDATE CALLBACK MUST NOT BE NULL");
        ret = cbUpdateInitConfig(params);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE INIT CONFIG DUE TO THE SECONDARY MEMORY MALFUNCTION");
            return ret;
        }
        
        File file = esp32FS.Open(OTA_REQUEST_PATH, "w", true);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA REQUEST PATH");
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }
        
        file.write(reinterpret_cast<const uint8_t*>(payload), strlen(payload) + 1);
        file.flush();
        file.close();

        file = esp32FS.Open(OTA_REQUEST_PATH, "r", false);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA REQUEST PATH");
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }

        const size_t size = file.size();
        char readback[size] = {'\0'};
        file.readBytes(readback, size);
        file.close();

        if (strcmp(readback, payload) != 0)
        {
            LOG_ERROR(logger, "FAILED TO WRITE OTA REQUEST PATH");
            ret = Status::Code::BAD_DEVICE_FAILURE;
            esp32FS.Remove(OTA_REQUEST_PATH);
            return ret;
        }

        uint8_t trialCount = 0;
        for (; trialCount < MAX_RETRY_COUNT; trialCount++)
        {
            ret = StopMqttTaskService();
            if (ret == Status::Code::GOOD)
            {
                break;
            }
        }
        
        if (trialCount == MAX_RETRY_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO STOP MQTT TASK");
        }
        LOG_INFO(logger, "MQTT service has been")

        LOG_INFO(logger,"Device will be reset due to OTA request");
    #if defined(MT10) || defined(MB10)
        spear.Reset();
    #endif
        esp_restart();
    }


    Status processMessageRemoteControl(const char* payload)
    {
        JSON json;
        remote_controll_struct_t messageconfig;
        std::string serializedPayload;
        JsonDocument doc;
        Status retJSON = json.Deserialize(payload, &doc);
        std::string Description;

        if (retJSON != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());

            switch (retJSON.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                Description = "PAYLOAD INSUFFICIENT OR INCOMPLETE";
                break;
            case Status::Code::BAD_NO_DATA:
                Description ="PAYLOAD EMPTY";
                break;
            case Status::Code::BAD_DATA_ENCODING_INVALID:
                Description = "PAYLOAD INVALID ENCODING";
                break;
            case Status::Code::BAD_OUT_OF_MEMORY:
                Description = "PAYLOAD OUT OF MEMORY";
                break;
            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                Description = "PAYLOAD EXCEEDED NESTING LIMIT";
                break;
            case Status::Code::BAD_UNEXPECTED_ERROR:
                Description = "UNDEFINED CONDITION";
                break;
            default:
                Description = "UNDEFINED CONDITION";
                break;
            }
            
            messageconfig.SourceTimestamp = GetTimestampInMillis(); 
            messageconfig.ResponseCode    = 900;
            messageconfig.Description = Description;

            serializedPayload = json.Serialize(messageconfig);
            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
            Status ret = mqtt::cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }
            return retJSON;
        }

        messageconfig.ID = doc["id"].as<uint32_t>();

        Status ret = Status(Status::Code::UNCERTAIN);

        bool isValid = true;
        isValid &= doc.containsKey("mc");
        isValid &= doc.containsKey("md");
        
        if (isValid == false)
        {
            messageconfig.ResponseCode = 900;
            messageconfig.Description = "NOT FOUND KEY : MC or MD";

            messageconfig.SourceTimestamp  = GetTimestampInMillis();
            serializedPayload = json.Serialize(messageconfig);
            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
            ret = mqtt::cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }

            return ret;
        }
        
        JsonArray mc = doc["mc"].as<JsonArray>();
        JsonArray md = doc["md"].as<JsonArray>();
        
   
        if (!mc.isNull() && mc.size() > 0) 
        {
            ret = RemoteControllToMachine(&messageconfig, mc);
        }    
        else if (!md.isNull() && md.size() > 0) 
        {
            ret = RemoteControllToModlink(&messageconfig, md);
        }


        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"REMOTE CONTROL ERROR ! %s",ret.c_str());
        }
        
        messageconfig.SourceTimestamp  = GetTimestampInMillis();
        serializedPayload = json.Serialize(messageconfig);
        mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
        ret = mqtt::cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
            /**
             * @todo Store 실패시 falsh 메모리에 저장하는 방법
             * 
             */
            LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
        }
        return ret;
    }

    Status subscribeMessages(init_cfg_t& params)
    {
        if (mqtt::cia.Count() == 0)
        {
            return Status(Status::Code::GOOD);
        }

        const std::pair<Status, mqtt::Message> message = mqtt::cia.Retrieve();
        if (message.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PEEK MESSAGE: %s ", message.first.c_str());
            return message.first;
        }
        
        Status ret(Status::Code::UNCERTAIN);
        switch (message.second.GetTopicCode())
        {
        case mqtt::topic_e::JARVIS_REQUEST:
            ret = processMessageJARVIS(params, message.second.GetPayload());
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Processed JARVIS request successfully");
            }
            else
            {
                LOG_ERROR(logger, "FAILED TO PROCESS JARVIS REQUEST MESSAGE: %s", ret.c_str());
            }
            return ret;

        case mqtt::topic_e::JARVIS_INTERFACE_REQUEST:
            return processMessageJarvisStatus(message.second.GetPayload());
        
        case mqtt::topic_e::FOTA_UPDATE:
            ret = processMessageUpdate(params, message.second.GetPayload());
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Processed update request successfully");
            }
            else
            {
                LOG_ERROR(logger, "FAILED TO PROCESS OTA REQUEST MESSAGE: %s", ret.c_str());
            }
            return ret;

        case mqtt::topic_e::REMOTE_CONTROL_REQUEST:
            return processMessageRemoteControl(message.second.GetPayload());
        
        default:
            ASSERT(false, "UNDEFINED TOPIC: 0x%02X", static_cast<uint8_t>(message.second.GetTopicCode()));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    void implMqttTask(void* pvParameters)
    {
        uint32_t statusReportMillis = millis(); 
        uint32_t reconnectMillis    = millis();
        
        init_cfg_t params = {
            .PanicResetCount   = reinterpret_cast<init_cfg_t*>(pvParameters)->PanicResetCount,
            .HasPendingJARVIS  = reinterpret_cast<init_cfg_t*>(pvParameters)->HasPendingJARVIS,
            .HasPendingUpdate  = reinterpret_cast<init_cfg_t*>(pvParameters)->HasPendingUpdate,
            .ReconfigCode      = reinterpret_cast<init_cfg_t*>(pvParameters)->ReconfigCode
        };

        while (true)
        {

        #if defined(DEBUG)
            if ((millis() - statusReportMillis) > (590 * SECOND_IN_MILLIS))
        #else
            if ((millis() - statusReportMillis) > (3600 * SECOND_IN_MILLIS))
        #endif

            {
                /**
                 * @todo 현재 DeviceStatus에 필요한 정보를 mqttTask에서 생성하고, COD로 넘겨주고 있음 추후에는 이 기능을 별도의 task로 빼서 구현해야함
                 * @김주성
                 * 
                 */
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
                size_t RemainedHeapSize = ESP.getFreeHeap();
                size_t RemainedFlashMemorySize = esp32FS.GetTotalBytes();
                LOG_DEBUG(logger, "[MqttTask] Stack Remaind: %u Bytes", RemainedStackSize);
                LOG_DEBUG(logger, "Heap Remained: %u Bytes", RemainedHeapSize);
                LOG_DEBUG(logger, "Flash Memory Remained: %u Bytes", RemainedFlashMemorySize);

                deviceStatus.SetTaskRemainedStack(task_name_e::MQTT_TASK, RemainedStackSize);
                deviceStatus.SetRemainedHeap(RemainedHeapSize);
                deviceStatus.SetRemainedFlash(RemainedFlashMemorySize);

                if(jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
                {
                    catm1_report_t signal;
                    if(catM1->GetSignalQuality(&signal) == Status(Status::Code::GOOD))
                    {
                        deviceStatus.SetReportCatM1(signal);
                    }
                }
                
                const std::string payload =  deviceStatus.ToStringCyclical();
                mqtt::Message message(mqtt::topic_e::JARVIS_STATUS, payload);
                mqtt::cdo.Store(message);

            }
            
            if (uint32_t(millis() - reconnectMillis) > (10 * SECOND_IN_MILLIS))
            {
                if (manageConnection() == Status::Code::BAD_NOT_EXECUTABLE)
                {
                    StopMqttTaskService();
                }
                reconnectMillis = millis();
            }
            
            publishMessages();
            subscribeMessages(params);
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
    }

    void implStopMqttTask(TimerHandle_t)
    {
        /**
         * @todo 전송하지 못한 메시지는 플래시 메모리에라도 저장해야 합니다.
         */
        if (xHandle == NULL)
        {
            return;
        }
        
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    Status StartMqttTaskService(init_cfg_t& config, CallbackUpdateInitConfig callbackJARVIS)
    {
        if (xHandle != NULL)
        {
            return Status(Status::Code::GOOD);
        }

        if (cbUpdateInitConfig == nullptr)
        {
            cbUpdateInitConfig = callbackJARVIS;
        }

        BaseType_t ret = xTaskCreatePinnedToCore(implMqttTask,     // Function to be run inside of the task
                                                 "implMqttTask",   // The identifier of this task for men
                                                 8*KILLOBYTE,	   // Stack memory size to allocate
                                                 &config,		   // Task parameters to be passed to the function
                                                 25,		       // Task Priority for scheduling
                                                 &xHandle,         // The identifier of this task for machines
                                                 1);			   // Index of MCU core where the function to run

        switch (ret)
        {
        case pdPASS:
            LOG_INFO(logger, "MQTT task has been started");
            return Status(Status::Code::GOOD);

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", static_cast<int>(ret));
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    Status StopMqttTaskService()
    {
        INetwork* snic = RetrieveServiceNicService();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }
        mqttClient->Disconnect(mutex.second);
        
        TimerHandle_t xTimer = xTimerCreate("implStopMqttTask",  // pcTimerName
                                            SECOND_IN_MILLIS,   // xTimerPeriod,
                                            pdFALSE,            // uxAutoReload,
                                            (void *)0,          // pvTimerID,
                                            implStopMqttTask);  // pxCallbackFunction
        
        if (xTimer == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE TIMER FOR STOPPING MQTT SERVICE");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        else
        {
            LOG_INFO(logger, "Created a timer for stopping mqtt service");
            if (xTimerStart(xTimer, 0) != pdPASS)
            {
                LOG_ERROR(logger, "FAILED TO START TIMER FOR STOPPING MQTT SERVICE");
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }
        
        LOG_INFO(logger, "Stopping the MQTT service");
        return Status(Status::Code::GOOD);
    }

    Status RemoteControllToMachine(remote_controll_struct_t* message, JsonArray& mc)
    {
        std::vector<std::pair<std::string, std::string>> remoteData;
        for (JsonObject obj : mc)
        {
            std::string nid = obj["nid"].as<std::string>(); 
            std::string value = obj["val"].as<std::string>(); 
            
            remoteData.emplace_back(nid, value); 
        }

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        
        std::pair<Status, im::Node*> ret = nodeStore.GetNodeReference(remoteData.at(0).first);
        std::pair<Status, uint16_t> retConvertModbus = std::make_pair(Status(Status::Code::UNCERTAIN),0);

        if (ret.first != Status::Code::GOOD)
        {
            message->SourceTimestamp   = GetTimestampInMillis();
            message->ResponseCode  = 900;
            message->Description = "UNDEFINED NODEID : " + remoteData.at(0).first;

            return ret.first;
        }

        retConvertModbus = ret.second->VariableNode.ConvertModbusData(remoteData.at(0).second);
        if (retConvertModbus.first != Status::Code::GOOD)
        {
            message->SourceTimestamp = GetTimestampInMillis();
            message->ResponseCode  = 900;
            message->Description = retConvertModbus.first.ToString();

           return retConvertModbus.first;
        }
        else
        {
            uint8_t writeResult = 0;
    #if defined(MT10) || defined(MB10) || defined(MT11)
            if (mConfigVectorMbTCP.size() != 0)
            {
                for (auto& TCP : mConfigVectorMbTCP)
                {
                    for (auto& modbusTCP : ModbusTcpVector)
                    {
                        if (modbusTCP.GetServerIP() != TCP.GetIPv4().second || modbusTCP.GetServerPort() != TCP.GetPort().second)
                        {
                            continue;
                        }
                        
                        std::pair<muffin::Status, std::vector<std::string>> retVector = TCP.GetNodes();
                        if (retVector.first == Status(Status::Code::GOOD))
                        {
                            for (auto& nodeId : retVector.second )
                            {
                                if (nodeId == ret.second->GetNodeID())
                                {   
                                    std::pair<muffin::Status, uint8_t> retSlaveID =  TCP.GetSlaveID();
                                    if (retSlaveID.first != Status(Status::Code::GOOD))
                                    {
                                        retSlaveID.second = 0;
                                    }
                                    jvs::node_area_e nodeArea = ret.second->VariableNode.GetNodeArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    int16_t retBit = ret.second->VariableNode.GetBitIndex();
                        
                                    if (retBit != -1)
                                    {
                                        modbus::datum_t registerData =  modbusTCP.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, nodeArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }
                                    
                                    writeResult = 0;
                                    if (xSemaphoreTake(xSemaphoreModbusTCP, 100000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MODBUS TCP] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto RC_RESPONSE;
                                    }

                                    LOG_DEBUG(logger, "[MODBUS TCP] 원격제어 : %u",retConvertModbus.second);
                                    switch (nodeArea)
                                    {
                                    case jvs::node_area_e::COILS:
                                        writeResult = modbusTCP.mModbusTCPClient->coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    case jvs::node_area_e::HOLDING_REGISTER:
                                        writeResult = modbusTCP.mModbusTCPClient->holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    default:
                                        LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", nodeArea);
                                        break;
                                    }

                                    xSemaphoreGive(xSemaphoreModbusTCP);

                                    goto RC_RESPONSE;
                                }
                                
                            }
                            
                        }
                    }

            #if defined(MT11)
                    for (auto& modbusTCP : ModbusTcpVectorDynamic)
                    {
                        if (modbusTCP.GetServerIP() != TCP.GetIPv4().second || modbusTCP.GetServerPort() != TCP.GetPort().second)
                        {
                            continue;
                        }
                        
                        std::pair<muffin::Status, std::vector<std::string>> retVector = TCP.GetNodes();
                        if (retVector.first == Status(Status::Code::GOOD))
                        {
                            for (auto& nodeId : retVector.second )
                            {
                                if (nodeId == ret.second->GetNodeID())
                                {   
                                    std::pair<muffin::Status, uint8_t> retSlaveID =  TCP.GetSlaveID();
                                    if (retSlaveID.first != Status(Status::Code::GOOD))
                                    {
                                        retSlaveID.second = 0;
                                    }
                                    jvs::node_area_e nodeArea = ret.second->VariableNode.GetNodeArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    int16_t retBit = ret.second->VariableNode.GetBitIndex();
                        
                                    if (retBit != -1)
                                    {
                                        modbus::datum_t registerData =  modbusTCP.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, nodeArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }
                                    
                                    writeResult = 0;
                                    if (xSemaphoreTake(xSemaphoreModbusTCP, 100000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MODBUS TCP] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto RC_RESPONSE;
                                    }

                                    if (modbusTCP.mModbusTCPClient->begin(modbusTCP.GetServerIP(), modbusTCP.GetServerPort()) != 1)
                                    {
                                        LOG_ERROR(logger,"Modbus TCP Client failed to connect!, serverIP : %s, serverPort: %d", modbusTCP.GetServerIP().toString().c_str(), modbusTCP.GetServerPort());
                                        goto RC_RESPONSE;
                                    }

                                    LOG_DEBUG(logger, "[MODBUS TCP] 원격제어 : %u",retConvertModbus.second);
                                    switch (nodeArea)
                                    {
                                    case jvs::node_area_e::COILS:
                                        writeResult = modbusTCP.mModbusTCPClient->coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    case jvs::node_area_e::HOLDING_REGISTER:
                                        writeResult = modbusTCP.mModbusTCPClient->holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    default:
                                        LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", nodeArea);
                                        break;
                                    }
                                    
                                    modbusTCP.mModbusTCPClient->end();

                                    xSemaphoreGive(xSemaphoreModbusTCP);

                                    goto RC_RESPONSE;
                                }
                            }
                        }
                    }
                #endif
                }
            }

            if (mConfigVectorMelsec.size() != 0)
            {
                for (auto& melsecConfig : mConfigVectorMelsec)
                {
                    for (auto& melsec : MelsecVector)
                    {
                        if (melsec.GetServerIP() != melsecConfig.GetIPv4().second || melsec.GetServerPort() != melsecConfig.GetPort().second)
                        {
                            continue;
                        }
                        
                        std::pair<muffin::Status, std::vector<std::string>> retVector = melsecConfig.GetNodes();
                        if (retVector.first == Status(Status::Code::GOOD))
                        {
                            for (auto& nodeId : retVector.second )
                            {
                                if (nodeId == ret.second->GetNodeID())
                                {   
                                    jvs::node_area_e nodeArea = ret.second->VariableNode.GetNodeArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    int16_t retBit = ret.second->VariableNode.GetBitIndex();

                                    if (retBit != -1)
                                    {
                                        modbus::datum_t registerData =  melsec.GetAddressValue(1, modbusAddress.Numeric, nodeArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }      
                                    
                                    writeResult = 0;
                                    if (xSemaphoreTake(xSemaphoreMelsec, 100000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MELSEC] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto RC_RESPONSE;
                                    }

                                    uint8_t MAX_TRIAL_COUNT = 3;
                                    uint8_t trialCount = 0;

                                    for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
                                    {
                                        if (melsec.Connect())
                                        {
                                            break;
                                        }

                                        LOG_WARNING(logger,"[#%d] melsec Client failed to connect!, serverIP : %s, serverPort: %d",trialCount, melsec.GetServerIP().toString().c_str(), melsec.GetServerPort());
                                        melsec.mMelsecClient->Close();
                                        delay(80);
                                    }

                                    if (trialCount == MAX_TRIAL_COUNT)
                                    {
                                        LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                                        xSemaphoreGive(xSemaphoreMelsec);
                                        goto RC_RESPONSE;
                                    }

                                    LOG_DEBUG(logger, "[MELSEC] 원격제어 : %u",retConvertModbus.second);
                                    
                                    if (im::IsBitArea(nodeArea))
                                    {
                                        LOG_DEBUG(logger,"AREA : %d, ADDRESS : %d",nodeArea, modbusAddress.Numeric);
                                        writeResult = melsec.mMelsecClient->WriteBit(nodeArea, modbusAddress.Numeric, retConvertModbus.second);
                                    }
                                    else
                                    {
                                        LOG_DEBUG(logger,"AREA : %d, ADDRESS : %d",nodeArea,modbusAddress.Numeric);
                                        writeResult = melsec.mMelsecClient->WriteWord(nodeArea, modbusAddress.Numeric, retConvertModbus.second);
                                    }

                                    melsec.mMelsecClient->Close();
                                    xSemaphoreGive(xSemaphoreMelsec);
                                    goto RC_RESPONSE;
                                }   
                            }       
                        }
                    }                    
                }
            }
    #endif

            if (mConfigVectorMbRTU.size() != 0)
            {
                for (auto& RTU : mConfigVectorMbRTU)
                {
                    for (auto& modbusRTU : ModbusRtuVector)
                    {
                        if (RTU.GetPort().second != modbusRTU.mPort)
                        {
                            continue;
                        }

                        std::pair<muffin::Status, std::vector<std::string>> retVector = RTU.GetNodes();
                        if (retVector.first == Status(Status::Code::GOOD))
                        {
                            for (auto& nodeId : retVector.second )
                            {
                                if (nodeId == ret.second->GetNodeID())
                                {
                                    std::pair<muffin::Status, uint8_t> retSlaveID =  RTU.GetSlaveID();
                                    if (retSlaveID.first != Status(Status::Code::GOOD))
                                    {
                                        retSlaveID.second = 0;
                                    }

                                    jvs::node_area_e nodeArea = ret.second->VariableNode.GetNodeArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    int16_t retBit = ret.second->VariableNode.GetBitIndex();
                        
                                    if (retBit != -1)
                                    {
                                        modbus::datum_t registerData =  modbusRTU.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, nodeArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }
                                    
                                    writeResult = 0;
                                #if defined(MODLINK_L) || defined(ML10) || defined(MT11)
                                    if (xSemaphoreTake(xSemaphoreModbusRTU, 100000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MODBUS RTU] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto RC_RESPONSE;
                                    }

                                    switch (nodeArea)
                                    {
                                    case jvs::node_area_e::COILS:
                                        writeResult = ModbusRTUClient.coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    case jvs::node_area_e::HOLDING_REGISTER:
                                        writeResult = ModbusRTUClient.holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    default:
                                        LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", nodeArea);
                                        break;
                                    }

                                    
                                    if (writeResult != 1)
                                    {
                                        if (ModbusRTUClient.lastError() != nullptr)
                                        {
                                            LOG_ERROR(logger,"FAIL TO REMOTE CONTROLL : %s",ModbusRTUClient.lastError());
                                            ModbusRTUClient.clearError();
                                        }
                                    }
                                    
                                    xSemaphoreGive(xSemaphoreModbusRTU);
                                    
                                    goto RC_RESPONSE;
                                #else
                                    spear_remote_control_msg_t msg;
                                    msg.Link = modbusRTU.mPort;
                                    msg.SlaveID = retSlaveID.second;
                                    msg.Area = nodeArea;
                                    msg.Address = modbusAddress.Numeric;
                                    msg.Value = retConvertModbus.second;
                                    Status result = spear.ExecuteService(msg);
                                    if (result == Status(Status::Code::GOOD))
                                    {
                                        writeResult = 1;
                                    }
                                #endif
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
RC_RESPONSE:
            if (writeResult == 1)
            {
                message->ResponseCode = 200;
            }
            else
            {
                message->ResponseCode  = 900;
                message->Description = "UNEXPECTED ERROR";
            }
            
            return Status(Status::Code::GOOD);
        }
    }

    Status RemoteControllToModlink(remote_controll_struct_t* message, JsonArray& md)
    {
        /**
         * @todo 1.4.0 에서는 하나의 설정 값만 입력 받는다. 
         * 
         */
        JsonObject obj = md[0]; // 추후 변경이 필요한 로직
   
        std::string nodeID = obj["nid"].as<std::string>();
        uint8_t limitType = obj["tp"].as<uint8_t>();
        std::string val = obj["val"].as<std::string>();
        
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        
        
        if (limitType == static_cast<uint8_t>(jvs::alarm_pub_type_e::UCL))
        {
            LOG_DEBUG(logger,"상한 알람 값 변경, %s",nodeID.c_str());
            Status result = alarmMonitor.ConvertUCL(nodeID, val);
            if (result == Status::Code::GOOD)
            {
                message->SourceTimestamp   = GetTimestampInMillis();
                message->ResponseCode      = 200;                
            }
            else
            {
                message->SourceTimestamp   = GetTimestampInMillis();
                message->ResponseCode    = 900;
                message->Description  = "FAIL TO SETING UCL";
            }
        }
        else if (limitType == static_cast<uint8_t>(jvs::alarm_pub_type_e::LCL))
        {
            LOG_DEBUG(logger,"하한 알람 값 변경, %s",nodeID.c_str());
            Status result = alarmMonitor.ConvertLCL(nodeID, val);
            if (result == Status::Code::GOOD)
            {
                message->SourceTimestamp   = GetTimestampInMillis();
                message->ResponseCode      = 200;                
            }
            else
            {
                message->SourceTimestamp   = GetTimestampInMillis();
                message->ResponseCode    = 900;
                message->Description  = "FAIL TO SETING LCL";
            }

        }
        else
        {
            message->SourceTimestamp   = GetTimestampInMillis();
            message->ResponseCode    = 900;
            message->Description  = "LIMIT TPYE ERROR : " + limitType;
        }

        return Status(Status::Code::GOOD);
    }
}