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
#include "Network/INetwork.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"



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
        mqtt::CatMQTT* catMQTT = new(std::nothrow) mqtt::CatMQTT(brokerInfo, lwt);
        if (catMQTT == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatM1 MQTT CLIENT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        const std::pair<Status, size_t> mutex = catM1->TakeMutex();
        Status ret = catMQTT->Init(mutex.second, network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized CatM1 MQTT Client");
            mqttClient = catMQTT;
        }
        else
        {
            delete catMQTT;
            catMQTT = nullptr;
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1 MQTT Client: %s", ret.c_str());
        }

        catM1->ReleaseMutex();
        return ret;
    }



    Status strategyInitEthernet()
    {
        mqtt::Message lwt = GenerateWillMessage(false);
        mqtt::LwipMQTT* lwipMQTT = new(std::nothrow) mqtt::LwipMQTT(brokerInfo, lwt);
        if (lwipMQTT == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR LwIP MQTT CLIENT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        Status ret = lwipMQTT->Init();
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized LwIP MQTT Client");
            mqttClient = lwipMQTT;
        }
        else
        {
            delete lwipMQTT;
            lwipMQTT = nullptr;
            LOG_ERROR(logger, "FAILED TO INITIALIZE LwIP MQTT Client: %s", ret.c_str());
        }

        return ret;
    }

    Status InitMqttService()
    {
        if (mqttClient != nullptr)
        {
            return Status(Status::Code::GOOD);
        }
        
        switch (jvs::config::operationCIN.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            return strategyInitCatM1();

        case jvs::snic_e::Ethernet:
            return strategyInitEthernet();
        
        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operationCIN.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
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
        if (jvs::config::operationCIN.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
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
        if (jvs::config::operationCIN.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            catM1->ReleaseMutex();
        }
        return ret;
    }
}