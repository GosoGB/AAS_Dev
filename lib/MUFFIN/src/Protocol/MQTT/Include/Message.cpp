/**
 * @file Message.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 메시지 클래스를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
 * 
 * @todo  기존과 달리 MAC 주소가 포함된 Topic 구조에 맞게 기존에 작성한 코드를 수정해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/MacAddress/MacAddress.h"
#include "Message.h"
#include "Topic.h"



namespace muffin { namespace mqtt {

    Message::Message()
        : mIsSocketIdSet(false)
        , mIsMessageIdSet(false)
        , mIsQosSet(false)
        , mIsRetainSet(false)
        , mIsTopicSet(false)
        , mIsPayloadSet(false)
    {
    #if defined(DEBUG)
        LOG_WARNING(logger, "CONSTRUCTED WITHOUT ANY DATA: %p", this);
    #endif
    }

    Message::Message(const topic_e topic, const std::string& payload, const socket_e socketID, const uint16_t messageID, const qos_e qos, const bool isRetain)
        : mIsSocketIdSet(true)
        , mIsMessageIdSet(true)
        , mIsQosSet(true)
        , mIsRetainSet(true)
        , mIsTopicSet(true)
        , mIsPayloadSet(true)
        , mSocketID(socketID)
        , mMessageID(messageID)
        , mQoS(qos)
        , mRetainFlag(isRetain)
        , mTopicCode(topic)
        , mPayload(payload)
    {
    #if defined(DEBUG)
        char buffer[1024] = {0};
        sprintf(buffer, "\n \
--------------------------------------------------\n \
Constructed at address: %p\n \
--------------------------------------------------\n \
SocketID: %u\n \
MessageID: %u\n \
QoS Level: %u\n \
Retain: %s\n \
Topic: %s\n \
Payload: %s\n \
--------------------------------------------------\n\n"
, this, static_cast<uint8_t>(GetSocketID()), GetMessageID(), static_cast<uint8_t>(GetQoS()),
IsRetain() ? "true" : "false", GetTopicString(), GetPayload());
        LOG_VERBOSE(logger, "%s", buffer);
    #endif
    }

    Message::Message(const Message& obj)
        : mIsSocketIdSet(obj.mIsSocketIdSet)
        , mIsMessageIdSet(obj.mIsMessageIdSet)
        , mIsQosSet(obj.mIsQosSet)
        , mIsRetainSet(obj.mIsRetainSet)
        , mIsTopicSet(obj.mIsTopicSet)
        , mIsPayloadSet(obj.mIsPayloadSet)
        , mSocketID(obj.mSocketID)
        , mMessageID(obj.mMessageID)
        , mQoS(obj.mQoS)
        , mRetainFlag(obj.mRetainFlag)
        , mTopicCode(obj.mTopicCode)
        , mPayload(obj.mPayload)
    {
    #if defined(DEBUG)
        char buffer[1024] = {0};
        sprintf(buffer, "\n \
--------------------------------------------------\n \
Constructed by Copy from %p to %p\n \
--------------------------------------------------\n \
SocketID: %u\n \
MessageID: %u\n \
QoS Level: %u\n \
Retain: %s\n \
Topic: %s\n \
Payload: %s\n \
--------------------------------------------------\n\n"
, &obj, this, static_cast<uint8_t>(GetSocketID()), GetMessageID(), static_cast<uint8_t>(GetQoS()),
IsRetain() ? "true" : "false", GetTopicString(), GetPayload());
        LOG_VERBOSE(logger, "%s", buffer);
    #endif
    }

    Message::Message(Message&& obj) noexcept
        : mIsSocketIdSet(std::move(obj.mIsSocketIdSet))
        , mIsMessageIdSet(std::move(obj.mIsMessageIdSet))
        , mIsQosSet(std::move(obj.mIsQosSet))
        , mIsRetainSet(std::move(obj.mIsRetainSet))
        , mIsTopicSet(std::move(obj.mIsTopicSet))
        , mIsPayloadSet(std::move(obj.mIsPayloadSet))
        , mSocketID(std::move(obj.mSocketID))
        , mMessageID(std::move(obj.mMessageID))
        , mQoS(std::move(obj.mQoS))
        , mRetainFlag(std::move(obj.mRetainFlag))
        , mTopicCode(std::move(obj.mTopicCode))
        , mPayload(std::move(obj.mPayload))
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "Constructed by Move from %p to %p", &obj, this);
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "SocketID: %u", mSocketID);
        LOG_DEBUG(logger, "MessageID: %u", mMessageID);
        LOG_DEBUG(logger, "QoS Level: %u", mQoS);
        LOG_DEBUG(logger, "Retain: %s", mRetainFlag ? "true" : "false");
        LOG_DEBUG(logger, "Topic: %s", Topic::ToString(mTopicCode));
        LOG_DEBUG(logger, "Payload: %s", mPayload.c_str());
        LOG_DEBUG(logger, "--------------------------------------------------\n\n");
    #endif
    }

    Message::~Message()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void Message::SetSocketID(const socket_e socketID)
    {
        mSocketID = socketID;
        mIsSocketIdSet = true;
    }

    void Message::SetMessageID(const uint16_t messageID)
    {
        mMessageID = messageID;
        mIsMessageIdSet = true;
    }

    void Message::SetQoS(const qos_e qosLevel)
    {
        mQoS = qosLevel;
        mIsQosSet = true;
    }

    void Message::SetRetain(const bool isRetain)
    {
        mRetainFlag = isRetain;
        mIsRetainSet = true;
    }

    void Message::SetTopic(const topic_e topicCode)
    {
        mTopicCode = topicCode;
        mIsTopicSet = true;
    }

    void Message::SetPayload(const std::string& payload)
    {
        mPayload = payload;
        mIsPayloadSet = true;
    }

    socket_e Message::GetSocketID() const
    {
        ASSERT((mIsSocketIdSet == true), "SOCKET ID NOT FOUND");
        return mSocketID;
    }

    uint16_t Message::GetMessageID() const
    {
        ASSERT((mIsMessageIdSet == true), "MESSAGE ID NOT FOUND");
        return mMessageID;
    }

    qos_e Message::GetQoS() const
    {
        ASSERT((mIsQosSet == true), "QoS LEVEL NOT FOUND");
        return mQoS;
    }

    bool Message::IsRetain() const
    {
        ASSERT((mIsRetainSet == true), "RETAIN FLAG NOT FOUND");
        return mRetainFlag;
    }

    topic_e Message::GetTopicCode() const
    {
        ASSERT((mIsTopicSet == true), "TOPIC CODE NOT FOUND");
        return mTopicCode;
    }

    const char* Message::GetTopicString() const
    {
        ASSERT((mIsTopicSet == true), "TOPIC CODE NOT FOUND");
        return Topic::ToString(mTopicCode);;
    }

    const char* Message::GetPayload() const
    {
        ASSERT((mIsPayloadSet == true), "PAYLOAD NOT FOUND");
        return mPayload.c_str();
    }
}}