/**
 * @file LwipMQTT.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief FreeRTOS TCP/IP 스택인 LwIP를 사용하는 MQTT 프로토콜 클래스를 정의합니다.
 * 
 * @date 2025-01-22
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo 향후 on-premise 방식으로 납품되는 경우, MQTTS 대신 MQTT 프로토콜로 
 *       연결해야 할 수도 있기 때문에 초기화 방식을 변경 가능하게 수정해야 함
 */





#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/Certs.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"



namespace muffin { namespace mqtt {

    Status LwipMQTT::Init()
    {
        xTimer = xTimerCreate(
            "lwip_mqtt_loop",   // pcTimerName
            1000,               // xTimerPeriod,
            pdTRUE,             // uxAutoReload,
            (void *)0,          // pvTimerID,
            vTimerCallback      // pxCallbackFunction
        );

        if (xTimer == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE TIMER FOR LOOP TASK");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        else
        {
            LOG_INFO(logger, "Created a timer for loop task");
            if (xTimerStart(xTimer, 0) != pdPASS)
            {
                LOG_ERROR(logger, "FAILED TO START TIMER FOR LOOP TASK");
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }
        
        
        mClient.setCallback(
            [this](char* topic, byte* payload, unsigned int length)
            {
                this->callback(topic, payload, length);
            }
        );
        LOG_INFO(logger, "Set a callback for subscription event");

        mNIC.setCACert(ROOT_CA_CRT);
        mClient.setClient(mNIC);
        mClient.setServer(mBrokerInfo.GetHost(),mBrokerInfo.GetPort());
        mClient.setKeepAlive(KEEP_ALIVE);
        const bool isBufferSizeSet = mClient.setBufferSize(BUFFER_SIZE);
        if (isBufferSizeSet == false)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR BUFFER");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        LOG_INFO(logger, "LwIP MQTT has been initialized");
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Connect(const size_t mutexHandle)
    {
        LOG_INFO(logger, "Start to connect to MQTT broker");
        mClient.connect(
            mBrokerInfo.GetClientID(),
            mBrokerInfo.GetUsername(),
            mBrokerInfo.GetPassword(),
            mMessageLWT.GetTopicString(),
            static_cast<uint8_t>(mMessageLWT.GetQoS()),
            mMessageLWT.IsRetain(),
            mMessageLWT.GetPayload()
        );
        
        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            if (mClient.connected() == true)
            {
                LOG_INFO(logger, "Connected to the Broker");
                goto CONNECTED;
            }
            LOG_WARNING(logger, "[TRIAL: #%u] NOT CONNECTED: %s", trialCount, getState());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        LOG_ERROR(logger, "FAILED TO CONNECT: %s", getState());
        return Status(Status::Code::BAD_NOT_CONNECTED);

    CONNECTED:
        if (mClient.publish(mMessageLWT.GetTopicString(), mqtt::GenerateWillMessage(true).GetPayload()) == true)
        {
            LOG_INFO(logger, "Published connection message");
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH CONNECTION MESSAGE");
        }
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Disconnect(const size_t mutexHandle)
    {
        mClient.disconnect();
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::IsConnected()
    {
        if (mClient.connected() == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD);
        }
    }

    Status LwipMQTT::Subscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        uint8_t trialCount = 0;

        for(const auto& message : messages)
        {
            for (trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
            {
                if (mClient.subscribe(message.GetTopicString()) == true)
                {
                    LOG_INFO(logger, "Subscribing: %s", message.GetTopicString());
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "[TRIAL: #%u] NOT SUBSCRIBED: %s", trialCount, getState());
                    continue;
                }
            }

            if (trialCount == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO SUBSCRIBE: %s", getState());
                return Status(Status::Code::BAD_COMMUNICATION_ERROR);
            }
        }

        LOG_INFO(logger, "Subscribed: %u topics", messages.size());
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        uint8_t trialCount = 0;

        for(const auto& message : messages)
        {
            for (trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
            {
                if (mClient.unsubscribe(message.GetTopicString()) == true)
                {
                    LOG_INFO(logger, "Unsubscribing: %s", message.GetTopicString());
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "[TRIAL: #%u] NOT UNSUBSCRIBED: %s", trialCount, getState());
                    continue;
                }
            }

            if (trialCount == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO UNSUBSCRIBE: %s", getState());
                return Status(Status::Code::BAD_COMMUNICATION_ERROR);
            }
        }

        LOG_INFO(logger, "Unsubscribed: %u topics", messages.size());
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Publish(const size_t mutexHandle, const Message& message)
    {
        uint8_t trialCount = 0;

        for (; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            if (mClient.publish(message.GetTopicString(), message.GetPayload()) == true)
            {
                return Status(Status::Code::GOOD);
            }
            else
            {
                LOG_WARNING(logger, "[TRIAL: #%u] NOT PUBLISHED: %s", trialCount, getState());
                continue;
            }
        }

        LOG_ERROR(logger, "FAILED TO PUBLISH: %s", getState());
        return Status(Status::Code::BAD_COMMUNICATION_ERROR);
    }

    INetwork* LwipMQTT::RetrieveNIC()
    {
        return static_cast<INetwork*>(ethernet);
    }

    const char* LwipMQTT::getState()
    {
        switch (mClient.state())
        {
        case MQTT_CONNECTION_TIMEOUT:
            return "CONNECTION TIMEOUT";
            
        case MQTT_CONNECTION_LOST:
            return "CONNECTION LOST";
            
        case MQTT_CONNECT_FAILED:
            return "CONNECTION FAILED";
            
        case MQTT_DISCONNECTED:
            return "DISCONNECTED";
            
        case MQTT_CONNECTED:
            return "CONNECTED";
            
        case MQTT_CONNECT_BAD_PROTOCOL:
            return "BAD PROTOCOL";
            
        case MQTT_CONNECT_BAD_CLIENT_ID:
            return "BAD CLIENT ID";
            
        case MQTT_CONNECT_UNAVAILABLE:
            return "UNAVAILABLE CONNECTION";
            
        case MQTT_CONNECT_BAD_CREDENTIALS:
            return "BAD CREDENTIALS";
            
        case MQTT_CONNECT_UNAUTHORIZED:
            return "UNAUTHORIZED CONNECTION";
            
        default:
            return nullptr;
        };
    }

    void LwipMQTT::vTimerCallback(TimerHandle_t xTimer)
    {
        configASSERT(xTimer);   // Optionally do something if xTimer is NULL
        implTimerCallback();
    }
    
    void LwipMQTT::implTimerCallback()
    {
        mClient.loop();
    }

    void LwipMQTT::callback(char* topic, byte* payload, unsigned int length)
    {
        const std::pair<bool, mqtt::topic_e> retTopic = mqtt::topic.ToCode(topic);
        if (retTopic.first == false)
        {
            LOG_ERROR(logger, "INVALID TOPIC RECEIVED: %s", topic);
            return;
        }

        std::string _payload(payload, payload + length);
        LOG_INFO(logger,"[Topic] %s\r\n[Payload] %s\r\n", topic, _payload.c_str());

        mqtt::Message message(retTopic.second, _payload);
        mqtt::cia.Store(message);
        return;
    }


    PubSubClient LwipMQTT::mClient;
}}