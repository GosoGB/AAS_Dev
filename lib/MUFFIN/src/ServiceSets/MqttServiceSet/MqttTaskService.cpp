/**
 * @file MqttTaskService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 태스크를 실행하고 정지하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
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
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"

static TaskHandle_t xHandle = NULL;
static uint8_t RECONNECT_TRIAL_COUNT = 0;



namespace muffin {

    INetwork* retrieveServiceNetwork()
    {
        const jvs::snic_e snicType = jvs::config::operation.GetServerNIC().second;

        switch (snicType)
        {
        case jvs::snic_e::LTE_CatM1:
            return static_cast<INetwork*>(catM1);

        case jvs::snic_e::Ethernet:
            return static_cast<INetwork*>(ethernet);
        
        default:
            ASSERT(false, "UNDEFINED SERVICE NETWORK TYPE: %u", static_cast<uint8_t>(snicType));
            return nullptr;
        }
    }

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
        
        INetwork* snic = retrieveServiceNetwork();
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

        INetwork* snic = retrieveServiceNetwork();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }

        Status ret(Status::Code::UNCERTAIN);
        for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
        {
            if (mqtt::cdo.Count() == 0)
            {
                return Status(Status::Code::GOOD);
            }
            
            const std::pair<Status, mqtt::Message> message = mqtt::cdo.Peek();
            if (message.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO PEEK MESSAGE: %s ", message.first.c_str());
                return message.first;
            }

            ret = mqttClient->Publish(mutex.second, message.second);
            if (ret == Status::Code::GOOD)
            {
                mqtt::cdo.Retrieve();
                continue;
            }

            LOG_WARNING(logger, "FAILED TO PUBLISH MESSAGE: %s", ret.c_str());
        }
        
        snic->ReleaseMutex();
        return ret;
    }

    jarvis_struct_t createJarvisResponse(Status::Code errorCode)
    {
        jarvis_struct_t response;

        switch (errorCode)
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

        return response;
    }

    Status processMessageJARVIS(const char* payload)
    {
        JSON json;
        JsonDocument doc;
        jarvis_struct_t response;
        Status ret = json.Deserialize(payload, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", ret.c_str());
            response = createJarvisResponse(ret.ToCode());
            response.SourceTimestamp = GetTimestampInMillis();
            goto ON_FAIL;
        }
        else
        {
            const std::pair<Status, jvs::prtcl_ver_e> retVersion = Convert.ToJarvisVersion(doc["ver"].as<const char*>());
            if ((retVersion.first.ToCode() != Status::Code::GOOD) || (retVersion.second > jvs::prtcl_ver_e::VERSEOIN_2))
            {
                LOG_ERROR(logger, "VERSION ERROR: %s , VERSION : %u", retVersion.first.c_str() ,static_cast<uint8_t>(retVersion.second));
                response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD_INVALID_VERSION);
                response.Description   = "INVALID OR UNSUPPORTED PROTOCOL VERSION";
            }
            ASSERT((retVersion.second == jvs::prtcl_ver_e::VERSEOIN_2), "ONLY JARVIS PROTOCOL VERSION 1 IS SUPPORTED");
            
            if (doc.containsKey("rqi") == true)
            {
                const char* rqi = doc["rqi"].as<const char*>();
                if (rqi == nullptr || strlen(rqi) == 0)
                {
                    response.ResponseCode  = Convert.ToUInt16(jvs::rsc_e::BAD);
                    response.Description   = "INVALID REQUEST ID: CANNOT BE NULL OR EMPTY";
                }
                ASSERT((rqi != nullptr || strlen(rqi) != 0), "REQUEST ID CANNOT BE NULL OR EMPTY");
            }
        }
    
    ON_FAIL:
        const std::string payload = json.Serialize(response);
        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);

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

    ON_SUCCESS:
        Preferences pf;
        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            if (pf.begin("jarvis") == false)
            {
                vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                continue;
            }

            pf.putChar("jarvisFlag", 1);
            const int8_t readback = pf.getChar("jarvisFlag", -1);

            if (readback != 1)
            {
                vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                continue;
            }
        }
        pf.end();
        LOG_INFO(logger,"Device will be reset due to JARVIS request");
        esp_restart();
        return ret;
    }

    Status processMessageRemoteControl(const char* payload)
    {
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status subscribehMessages()
    {
        if (mqtt::cia.Count() == 0)
        {
            return Status(Status::Code::GOOD);
        }

        const std::pair<Status, mqtt::Message> message = mqtt::cia.Peek();
        if (message.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PEEK MESSAGE: %s ", message.first.c_str());
            return message.first;
        }

        Status ret(Status::Code::UNCERTAIN);
        switch (message.second.GetTopicCode())
        {
        case mqtt::topic_e::JARVIS_REQUEST:
            return processMessageJARVIS(message.second.GetPayload());

        case mqtt::topic_e::REMOTE_CONTROL_REQUEST:
            return processMessageRemoteControl(message.second.GetPayload());

        case mqtt::topic_e::FOTA_UPDATE:
            // saveFotaFlag(payload);
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);

        default:
            ASSERT(false, "UNDEFINED TOPIC: 0x%02X", static_cast<uint8_t>(message.second.GetTopicCode()));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    void implMqttTask(void* pvParameters)
    {
        uint32_t reconnectMillis  = millis();

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
            subscribehMessages();
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
    }

    void implStopMqttTask(TimerHandle_t)
    {
        /**
         * @todo 전송하지 못한 메시지는 플래시 메모리에라도 저장해야 합니다.
         */
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    Status StartMqttTaskService()
    {
        if (xHandle != NULL)
        {
            return Status(Status::Code::GOOD);
        }

        BaseType_t ret = xTaskCreatePinnedToCore(implMqttTask,     // Function to be run inside of the task
                                                 "implMqttTask",   // The identifier of this task for men
                                                 4*KILLOBYTE,	   // Stack memory size to allocate
                                                 NULL,			   // Task parameters to be passed to the function
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