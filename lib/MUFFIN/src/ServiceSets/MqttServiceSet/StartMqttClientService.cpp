/**
 * @file StartMqttClientService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 클라이언트 초기화 및 연결하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
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
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"



namespace muffin {

    mqtt::BrokerInfo brokerInfo;

    mqtt::Message GenerateWillMessage(const bool isConnected)
    {
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

    #if defined(MODLINK_L) || defined(MODLINK_ML10) || defined(MT11)
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

    static Status strategyInitEthernet()
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

    Status subscribeTopics(const size_t mutex)
    {
        mqtt::Message jarvis(mqtt::topic_e::JARVIS_REQUEST, "");
        mqtt::Message jarvisStatus(mqtt::topic_e::JARVIS_INTERFACE_REQUEST, "");
        mqtt::Message remoteControl(mqtt::topic_e::REMOTE_CONTROL_REQUEST, "");
        mqtt::Message firmwareUpdate(mqtt::topic_e::FOTA_UPDATE, "");

        std::vector<mqtt::Message> topics;
        try
        {
            topics.reserve(4);
            topics.emplace_back(std::move(jarvis));
            topics.emplace_back(std::move(jarvisStatus));
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

    Status InitMqttClientService()
    {
        if (mqttClient != nullptr)
        {
            if (mqttClient->IsConnected() == Status::Code::GOOD)
            {
                return Status(Status::Code::GOOD);
            }
        }

        mqtt::topic.Init();
        brokerInfo.SetClientID(macAddress.GetEthernet());

        switch (jvs::config::operation.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            return strategyInitCatM1();

    #if defined(MT10) || defined(MB10) || defined(MT11)
        case jvs::snic_e::Ethernet:
            return strategyInitEthernet();
    #endif

        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operation.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status ConnectMqttClientService()
    {
        ASSERT((mqttClient != nullptr), "MQTT CLIENT MUST EXIST");

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
                if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
                {
                    catM1->ReleaseMutex();
                }
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