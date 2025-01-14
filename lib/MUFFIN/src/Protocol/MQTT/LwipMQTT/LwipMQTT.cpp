/**
 * 
 */




#include "LwipMQTT.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Network/Helper.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/Certs.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/CIA.h"




namespace muffin { namespace mqtt {

    LwipMQTT* LwipMQTT::CreateInstanceOrNULL(BrokerInfo& broker, Message& lwt)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) LwipMQTT(broker, lwt);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR LwipMQTT");
                return mInstance;
            }
        }
        
        return mInstance;
    }

    LwipMQTT* LwipMQTT::CreateInstanceOrNULL(BrokerInfo& broker)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) LwipMQTT(broker);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR LwipMQTT");
                return mInstance;
            }
        }
        
        return mInstance;
    }

    LwipMQTT& LwipMQTT::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    LwipMQTT::LwipMQTT(BrokerInfo& broker, Message& lwt)
        : mBrokerInfo(std::move(broker))
        , mMessageLWT(std::move(lwt))
        , xSemaphore(NULL)
    {
        mInitFlags.reset();
        mInitFlags.set(init_flag_e::ENABLE_LWT_MSG);
        mState = state_e::CONSTRUCTED;
    }
    
    LwipMQTT::LwipMQTT( BrokerInfo& broker)
        : mBrokerInfo(std::move(broker))
        , mMessageLWT(Message(topic_e::LAST_WILL, std::string()))
        , xSemaphore(NULL)
    {
        mInitFlags.reset();
        mInitFlags.reset(init_flag_e::ENABLE_LWT_MSG);
        mState = state_e::CONSTRUCTED;
        LOG_WARNING(logger, "LWT FEATURE IS TURNED OFF");
    }

    LwipMQTT::~LwipMQTT()
    {
    }

    void LwipMQTT::OnEventReset()
    {
        this->mInitFlags.reset();
    }

    void LwipMQTT::callback(char* topic, byte * payload, unsigned int length)
    {
        const auto retTopic = mqtt::Topic::ToCode(topic);
        if (retTopic.first == false)
        {
            LOG_ERROR(logger, "INVALID TOPIC: %s", topic);
            return;
        }

        std::string payloads;
        for (int i=0;i<length;i++) 
        {
            Serial.print((char)payload[i]);
            payloads = payloads + (char)payload[i];
        }

        const mqtt::topic_e topicCode = retTopic.second;
        mqtt::Message message(topicCode, payloads);
        mqtt::CIA::Store(message);
        LOG_INFO(logger,"PAYLOAD : %s",payloads.c_str());
        return;
    }

    Status LwipMQTT::Init()
    {
        xSemaphore = xSemaphoreCreateMutex();
        if (xSemaphore == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE SEMAPHORE");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        mLwipSecureClient.setCACert(certificates);
        mPubSubClient.setClient(mLwipSecureClient);
        mPubSubClient.setServer(mBrokerInfo.GetHost(),mBrokerInfo.GetPort());
        mPubSubClient.setKeepAlive(10);
        mPubSubClient.setBufferSize(10240);
        
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Connect(const size_t mutexHandle)
    {
       
        /**
         * @todo 연결 끊어졌을 때 상태를 다시 초기화해야 합니다. 그 다음 아래 assert를 다시 활성화시켜야 합니다.
         */
        // ASSERT((mState == state_e::INITIALIZED), "MUST BE INITIALIZED PRIOR TO \"Connect()\"");

        if (mState == state_e::CONNECTED)
        {
            LOG_WARNING(logger, "ALREADY CONNECTED. NOTHING TO DO");
            return Status(Status::Code::GOOD);
        }
        
        Status ret = connectBroker(mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO MQTT BROKER: %s", ret.c_str());
            return ret;
        }
        
        LOG_INFO(logger, "Connected to broker: %s", ret.c_str());
        mState = state_e::CONNECTED;
        return ret;
    }

    Status LwipMQTT::Disconnect(const size_t mutexHandle)
    {

        Status ret = disconnectBroker(mutexHandle);
        
        LOG_INFO(logger, "Disconnected from broker: %s", ret.c_str());
        mState = state_e::DISCONNECTED;
        return ret;
    }

    Status LwipMQTT::IsConnected()
    {
        mPubSubClient.loop();

        if (mPubSubClient.connected() == true)
        {
            mState = state_e::CONNECTED; 
            return Status(Status::Code::GOOD);
        }
        else
        {
            mState = state_e::DISCONNECTED; 
            return Status(Status::Code::BAD);
        }
    }

    Status LwipMQTT::Subscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        for(auto& message : messages)
        {
            if (mPubSubClient.subscribe(message.GetTopicString()) == false)
            {
                LOG_ERROR(logger, "FAILED TO SUBSCRIBE MQTT TOPICS: %s", message.GetTopicString());
                return Status(Status::Code::BAD);
            }
            else
            {
                LOG_INFO(logger, "SUCSESS TO SUBSCRIBE MQTT TOPICS: %s", message.GetTopicString());
            }
        }
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        for(auto& message : messages)
        {
            if (mPubSubClient.unsubscribe(message.GetTopicString()) == false)
            {
                LOG_ERROR(logger, "FAILED TO SUBSCRIBE MQTT TOPICS: %s", message.GetTopicString());
                return Status(Status::Code::BAD);
            }
            else
            {
                LOG_INFO(logger, "SUCSESS TO SUBSCRIBE MQTT TOPICS: %s", message.GetTopicString());
            }
        }
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::Publish(const size_t mutexHandle, const Message& message)
    {
        if (mPubSubClient.publish(message.GetTopicString(),message.GetPayload()))
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD);
        }
    }


    Status LwipMQTT::setLastWill(const size_t mutexHandle)
    {
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::setKeepAlive(const size_t mutexHandle)
    {
        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::connectBroker(const size_t mutexHandle)
    {
        do
        {
            Serial.print(".");
            delay(1000);
        } while (mPubSubClient.connect(mBrokerInfo.GetClientID(),mBrokerInfo.GetUsername(),
        mBrokerInfo.GetPassword(), mMessageLWT.GetTopicString(),0,false,mMessageLWT.GetPayload()) == false);
        
        LOG_DEBUG(logger, "Connected to the broker, payload : %s",mMessageLWT.GetPayload());

        if (mPubSubClient.publish(mMessageLWT.GetTopicString(),"1097BD14F34B,connected"))
        {
            LOG_DEBUG(logger, "GOOD");
        }

        mPubSubClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
            this->callback(topic, payload, length);
        });

        return Status(Status::Code::GOOD);
    }

    Status LwipMQTT::disconnectBroker(const size_t mutexHandle)
    {
        mPubSubClient.disconnect();

        
        return Status(Status::Code::GOOD);
    }

    std::pair<Status, size_t> LwipMQTT::TakeMutex()
    {
        if (xSemaphoreTake(xSemaphore, 5000)  != pdTRUE)
        {
            LOG_WARNING(logger, "FAILED TO TAKE MUTEX FOR LWIP TRY LATER.");
            return std::make_pair(Status(Status::Code::BAD_TOO_MANY_OPERATIONS), mMutexHandle);
        }

        ++mMutexHandle;
        return std::make_pair(Status(Status::Code::GOOD), mMutexHandle);
    }

    Status LwipMQTT::ReleaseMutex()
    {
        xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }
    
    

    LwipMQTT* LwipMQTT::mInstance = nullptr;
}}