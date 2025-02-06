/**
 * @file MqttTaskService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 태스크를 실행하고 정지하는 서비스를 정의합니다.
 * 
 * @date 2025-01-28
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <Preferences.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "DataFormat/JSON/JSON.h"
#include "JARVIS/JARVIS.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "JARVIS/Config/Network/CatM1.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
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
        LOG_WARNING(logger, "MQTT CLIENT IS NOT CONNECTED");
        
        INetwork* snic = RetrieveServiceNicService();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }
        
        Status ret = mqttClient->Disconnect(mutex.second);
        if (ret != Status::Code::GOOD)
        {
            goto ON_FAIL;
        }
        
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
        return ret;
        
    ON_FAIL:
        if (++RECONNECT_TRIAL_COUNT == MAX_RETRY_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO RECONNECT TO THE BROKER");
            LOG_INFO(logger, "Restart the device");
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
        if ((version.first.ToCode() != Status::Code::GOOD) || (version.second > jvs::prtcl_ver_e::VERSEOIN_2))
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
            params.HasPendingJARVIS = true;
            ASSERT((cbUpdateInitConfig != nullptr), "INIT CONFIG UPDATE CALLBACK MUST NOT BE NULL");
            ret = cbUpdateInitConfig(params);
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger,"Device will be reset due to JARVIS request");
            #if defined(MODLINK_T2) || defined(MODLINK_B)
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
        response->Ethernet.EnableDHCP = jvs::config::ethernet->GetDHCP().second;
        if (response->Ethernet.EnableDHCP == false)
        {
            response->Ethernet.StaticIPv4   = jvs::config::ethernet->GetStaticIPv4().second;
            response->Ethernet.Subnetmask   = jvs::config::ethernet->GetSubnetmask().second;
            response->Ethernet.Gateway      = jvs::config::ethernet->GetGateway().second;
            response->Ethernet.DNS1         = jvs::config::ethernet->GetDNS1().second;
            response->Ethernet.DNS2         = jvs::config::ethernet->GetDNS2().second;
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
        mqtt::Message message(mqtt::topic_e::JARVIS_STATUS_RESPONSE, std::move(ResponsePayload));

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
            #if defined(MODLINK_T2) || defined(MODLINK_B)
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
        
        LOG_INFO(logger,"Device will be reset due to OTA request");
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        spear.Reset();
    #endif
        esp_restart();
    }

    Status processMessageRemoteControl(const char* payload)
    {
        while (true)
        {
            LOG_ERROR(logger, "BAD_SERVICE_UNSUPPORTED");
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
        
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
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

        case mqtt::topic_e::JARVIS_STATUS_REQUEST:
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
            ret = processMessageRemoteControl(message.second.GetPayload());
            return ret;

        default:
            ASSERT(false, "UNDEFINED TOPIC: 0x%02X", static_cast<uint8_t>(message.second.GetTopicCode()));
            ret = Status(Status::Code::BAD_INVALID_ARGUMENT);
            return ret;
        }
    }

    void implMqttTask(void* pvParameters)
    {
        uint32_t reconnectMillis  = millis();
        init_cfg_t params = {
            .PanicResetCount   = reinterpret_cast<init_cfg_t*>(pvParameters)->PanicResetCount,
            .HasPendingJARVIS  = reinterpret_cast<init_cfg_t*>(pvParameters)->HasPendingJARVIS,
            .HasPendingUpdate  = reinterpret_cast<init_cfg_t*>(pvParameters)->HasPendingUpdate
        };

        while (true)
        {
            if ((millis() - reconnectMillis) > (10 * SECOND_IN_MILLIS))
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
                                                 5*KILLOBYTE,	   // Stack memory size to allocate
                                                 &config,		   // Task parameters to be passed to the function
                                                 0,				   // Task Priority for scheduling
                                                 &xHandle,         // The identifier of this task for machines
                                                 0);			   // Index of MCU core where the function to run

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
}