/**
 * @file LwipMQTT.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief FreeRTOS TCP/IP 스택인 LwIP를 사용하는 MQTT 프로토콜 클래스를 정의합니다.
 * 
 * @todo 향후 on-premise 방식으로 납품되는 경우, MQTTS 대신 MQTT 프로토콜로 
 *       연결해야 할 수도 있기 때문에 초기화 방식을 변경 가능하게 수정해야 함
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */





#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
// #include "Network/Ethernet/Ethernet.h"
#include "Protocol/Certs.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"



namespace muffin { namespace mqtt {

    TaskHandle_t xLwipMqttHandle = nullptr;
    
    static bool startTask = false;

    void LwipMQTT::implLwipMqttTask(void* pvParameter)
    {
        LwipMQTT* mqtt = static_cast<LwipMQTT*>(pvParameter);
        while (true)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            if (!startTask)
            {
                continue;
            }
            mqtt->mClient.loop();
        }
    }



    Status LwipMQTT::Init()
    {
        #if defined(MT11)
            mNIC = new w5500::EthernetClient(*ethernet,w5500::sock_id_e::SOCKET_6);
        #else
            mNIC = new WiFiClient();
        #endif

        mClient.setCallback(
            [this](char* topic, byte* payload, unsigned int length)
            {
                this->callback(topic, payload, length);
            }
        );
        LOG_INFO(logger, "Set a callback for subscription event");
        log_d("Remained Heap: %u Bytes", ESP.getFreeHeap());
        
    #if defined(MT11)
        if(mBrokerInfo.IsSslEnabled() == true)
        {
            if (mSecureNIC == nullptr)
            {
                mSecureNIC = new SSLClient(mNIC);
            }

            if (mBrokerInfo.IsValidateCert() == true)
            {
                mSecureNIC->setCACert(ROOT_CA_CRT);
            }
            else
            {
                mSecureNIC->setInsecure();
            }
            mClient.setClient(*mSecureNIC);
        }
        else
        {
            mClient.setClient(*mNIC);
        }
    #else
        if(mBrokerInfo.IsSslEnabled() == true)
        {
            mSecureNIC = new WiFiClientSecure();
            if (mBrokerInfo.IsValidateCert() == true)
            {
                mSecureNIC->setCACert(ROOT_CA_CRT);
            }
            else
            {
                mSecureNIC->setInsecure();
            }
            mClient.setClient(*mSecureNIC);
        }
        else
        {
            mClient.setClient(*mNIC);
        }
    #endif
    
        mClient.setServer(mBrokerInfo.GetHost(),mBrokerInfo.GetPort());
        mClient.setKeepAlive(KEEP_ALIVE);
     
        const bool isBufferSizeSet = mClient.setBufferSize(BUFFER_SIZE);
        if (isBufferSizeSet == false)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR BUFFER");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        if (xLwipMqttHandle != nullptr && eTaskGetState(xLwipMqttHandle) != eDeleted) 
        {
            return Status(Status::Code::GOOD);  // 이미 실행 중이면 OK 처리
        }

        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implLwipMqttTask,      // Function to be run inside of the task
            "implLwipMqttTask",    // The identifier of this task for men
    #if defined(MT11)
            4 * KILLOBYTE,          // Stack memory size to allocate
    #else
            4 * KILLOBYTE,          // Stack memory size to allocate
    #endif
            this,                   // Task parameters to be passed to the function
            2,				        // Task Priority for scheduling
            &xLwipMqttHandle,       // The identifier of this task for machines
            1				        // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The LwIP MQTT task has been started");
            break;

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            return Status(Status::Code::BAD);

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            return Status(Status::Code::BAD);

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            return Status(Status::Code::BAD);
        }

        log_d("Remained Heap: %u Bytes", ESP.getFreeHeap());
        LOG_INFO(logger, "LwIP MQTT has been initialized");
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Connect(const size_t mutexHandle)
    {
        log_d("Remained Heap: %u Bytes", ESP.getFreeHeap());
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
                startTask = true;
                return Status(Status::Code::GOOD);
            }
            LOG_WARNING(logger, "[TRIAL: #%u] NOT CONNECTED: %s", trialCount, getState());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        LOG_ERROR(logger, "FAILED TO CONNECT: %s", getState());
        return Status(Status::Code::BAD_NOT_CONNECTED);
    }

    Status LwipMQTT::Disconnect(const size_t mutexHandle)
    {
        mClient.disconnect();
        startTask = false;
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
        LOG_DEBUG(logger,"paylaod : %s",message.GetPayload());
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

    void LwipMQTT::callback(char* topic, byte* payload, unsigned int length)
    {
        const std::pair<bool, mqtt::topic_e> retTopic = mqtt::topic.ToCode(topic);
        if (retTopic.first == false)
        {
            LOG_ERROR(logger, "INVALID TOPIC RECEIVED: %s", topic);
            return;
        }

        std::string _payload(payload, payload + length);
        mqtt::Message message(retTopic.second, _payload);
        mqtt::cia.Store(message);
        return;
    }


    /**
     * @btodo [담당자] 김주성 전임연구원
     *        [작업 상세]
     *            임시적으로 만들어 둔 함수로 보입니다.
     *            필요 없는 경우 삭제, 필요한 경우 구현할
     *            내용 작성 요망
     */
    /**
     * @todo 해당 함수는 CatM1 모듈 리셋을 위한 함수이며 override을 위해 LWIP에 임시로 구현해놓은 코드입니다. 
     * 현재는 함수만 명시적으로 구현되어있는 상태이며 추후에 삭제할 예정입니다.
     * 
     */
    Status LwipMQTT::ResetTEMP()
    {
        return Status(Status::Code::GOOD);
    }

    PubSubClient LwipMQTT::mClient;
}}