/**
 * @file StartMqttClientService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 클라이언트 초기화 및 연결하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"

static TaskHandle_t xHandle = NULL;
static uint8_t RECONNECT_TRIAL_COUNT = 0;



namespace muffin {

    mqtt::BrokerInfo brokerInfo(
        macAddress.GetEthernet(),           // clientID
        "mqtt.vitcon.iotops.opsnow.com",    // host
        8883,                               // port
        7,                                  // keepalive
        mqtt::socket_e::SOCKET_0,           // socketID
        "vitcon",                           // username
        "tkfkdgo5!@#$"                      // password
    );

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
        
        Status ret = mqttClient->Disconnect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DISCONNECT: %s", ret.c_str());
            return ret;
        }
        
        if (implInitMqttService() != Status::Code::GOOD)
        {
            ++RECONNECT_TRIAL_COUNT;
            return;
        }

        if (RECONNECT_TRIAL_COUNT == MAX_RETRY_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO RECONNECT TO THE BROKER");
            LOG_INFO(logger, "Restart the device");
            esp_restart();
        }
    }

    void implStopMqttTask(TimerHandle_t)
    {
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    Status StopMqttService()
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

    void implMqttTask(void* pvParameters)
    {
        uint32_t reconnectMillis  = millis();

        while (true)
        {
            if ((millis() - reconnectMillis) > (10 * SECOND_IN_MILLIS))
            {
                if (manageConnection() == Status::Code::BAD_NOT_EXECUTABLE)
                {
                    StopMqttService();
                }
                reconnectMillis = millis();
            }
            
            cia 와 cdo 작업 이어서 해야 함;
            
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
    }

    Status startMqttTask()
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

    mqtt::Message GenerateWillMessage(const bool isConnected)
    {
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        snprintf(buffer, size, "%s,%llu,%s,%s,null",
            macAddress.GetEthernet(),
            GetTimestampInMillis(),
            isConnected ? "true" : "false",
            FW_VERSION_ESP32.GetSemanticVersion()
        );
    #else
        snprintf(buffer, size, "%s,%llu,%s,%s,%s",
            macAddress.GetEthernet(),
            GetTimestampInMillis(),
            isConnected ? "true" : "false",
            FW_VERSION_ESP32.GetSemanticVersion(),
            FW_VERSION_MEGA2560.GetSemanticVersion()
        );
    #endif
        
        return mqtt::Message(mqtt::topic_e::LAST_WILL, buffer);
    }

    Status strategyInitCatM1()
    {
        mqtt::Message lwt = GenerateWillMessage(false);
        mqtt::CatMQTT* catMQTT = nullptr;

        if (mqttClient == nullptr)
        {
            catMQTT = new(std::nothrow) mqtt::CatMQTT(brokerInfo, lwt);
            if (catMQTT == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatM1 MQTT CLIENT");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }
        else
        {
            catMQTT = static_cast<mqtt::CatMQTT*>(mqttClient);
        }
        
        const std::pair<Status, size_t> mutex = catM1->TakeMutex();
        Status ret = catMQTT->Init(mutex.second, network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized CatM1 MQTT Client");
            if (mqttClient == nullptr)
            {
                mqttClient = catMQTT;
            }
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1 MQTT Client: %s", ret.c_str());
        }

        catM1->ReleaseMutex();
        return ret;
    }

    Status strategyInitEthernet()
    {
        mqtt::Message lwt = GenerateWillMessage(false);
        mqtt::LwipMQTT* lwipMQTT = nullptr;

        if (mqttClient == nullptr)
        {
            lwipMQTT = new(std::nothrow) mqtt::LwipMQTT(brokerInfo, lwt);
            if (lwipMQTT == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatM1 MQTT CLIENT");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }
        else
        {
            lwipMQTT = static_cast<mqtt::LwipMQTT*>(mqttClient);
        }

        Status ret = lwipMQTT->Init();
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized LwIP MQTT Client");
            if (mqttClient == nullptr)
            {
                mqttClient = lwipMQTT;
            }
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE LwIP MQTT Client: %s", ret.c_str());
        }

        return ret;
    }

    Status implInitMqttService()
    {
        switch (jvs::config::operation.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            return strategyInitCatM1();

        case jvs::snic_e::Ethernet:
            return strategyInitEthernet();
        
        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operation.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status InitMqttService()
    {
        if (mqttClient != nullptr)
        {
            return Status(Status::Code::GOOD);
        }

        Status ret = startMqttTask();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        return implInitMqttService();
    }

    Status subscribeTopics(const size_t mutex)
    {
        mqtt::Message jarvis(mqtt::topic_e::JARVIS_REQUEST, "");
        mqtt::Message remoteControl(mqtt::topic_e::REMOTE_CONTROL_REQUEST, "");
        mqtt::Message firmwareUpdate(mqtt::topic_e::FOTA_UPDATE, "");

        std::vector<mqtt::Message> topics;
        try
        {
            topics.reserve(3);
            topics.emplace_back(std::move(jarvis));
            topics.emplace_back(std::move(remoteControl));
            topics.emplace_back(std::move(firmwareUpdate));
        }
        catch(const std::bad_alloc& e)
        {
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        return mqttClient->Subscribe(mutex, topics);
    }

    Status ConnectMqttService()
    {
        if (mqttClient->IsConnected() == Status::Code::GOOD)
        {
            return Status(Status::Code::GOOD);
        }
        
        std::pair<Status, size_t> mutex = std::make_pair(Status(Status::Code::BAD), 0);
        if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            mutex = catM1->TakeMutex();
            if (mutex.first != Status::Code::GOOD)
            {
                return mutex.first;
            }
        }
        
        mqtt::Message lwt = GenerateWillMessage(true);
        Status ret = mqttClient->Connect(mutex.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO THE MQTT BROKER: %s", ret.c_str());
            goto ON_FAIL;
        }
        LOG_INFO(logger, "Connected to the MQTT broker");

        ret = mqttClient->Publish(mutex.second, lwt);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH LAST WILL MESSAGE: %s", ret.c_str());
            goto ON_FAIL;
        }
        LOG_INFO(logger, "Published last will message");

        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            ret = subscribeTopics(mutex.second);
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Subscribed topics successfully");
                return ret;
            }

            LOG_WARNING(logger, "[TRIAL: #%u] SUBSCRIPTION WAS UNSUCCESSFUL: %s",
                trialCount, ret.c_str());
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
        LOG_ERROR(logger, "FAILED TO SUBSCRIBE TOPICS: %s", ret.c_str());

    ON_FAIL:
        if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            catM1->ReleaseMutex();
        }
        return ret;
    }
}