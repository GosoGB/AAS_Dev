/**
 * @file Message.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 메시지 클래스를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Message.h"



namespace muffin { namespace mqtt {

    Message::Message(const std::string& mac, topic_e topic, const std::string& payload, const socket_e socketID, const uint16_t messageID, const qos_e qos, const bool isRetain)
        : mMacAddress(mac)
        , mSocketID(socketID)
        , mMessageID(messageID)
        , mQoS(qos)
        , mRetainFlag(isRetain)
        , mTopicCode(topic)
        , mPayload(payload)
    {
        /**
         * @todo  기존과 달리 MAC 주소가 포함된 Topic 구조에 맞게 기존에 작성한 코드를 수정해야 합니다.
         */
        switch (mTopicCode)
        {
        case topic_e::LAST_WILL:
            mTopicString = "scautr/modlink/status/network/will";
            break;
        case topic_e::JARVIS_REQUEST:
            mTopicString = "mfm/" + mMacAddress;
            break;
        case topic_e::JARVIS_RESPONSE:
            mTopicString = "mfm/resp/" + mMacAddress;
            break;
        default:
            ASSERT(false, "UNDEFINED MQTT TOPIC");
            break;
        }

    #if defined(DEBUG)
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "Constructed at address: %p", this);
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "SocketID: %u", GetSocketID());
        LOG_DEBUG(logger, "MessageID: %u", GetMessageID());
        LOG_DEBUG(logger, "QoS Level: %u", GetQoS());
        LOG_DEBUG(logger, "Retain: %s", IsRetain() ? "true" : "false");
        LOG_DEBUG(logger, "Topic: %s", GetTopicString());
        LOG_DEBUG(logger, "Payload: %s", GetPayload());
        LOG_DEBUG(logger, "--------------------------------------------------\n\n");
    #endif
    }

    Message::Message(const Message& obj)
        : mMacAddress(obj.mMacAddress)
        , mSocketID(obj.mSocketID)
        , mMessageID(obj.mMessageID)
        , mQoS(obj.mQoS)
        , mRetainFlag(obj.mRetainFlag)
        , mTopicCode(obj.mTopicCode)
        , mPayload(obj.mPayload)
        , mTopicString(obj.mTopicString)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "--------------------------------------------------");
        LOG_VERBOSE(logger, "Constructed by Copy from %p to %p", &obj, this);
        LOG_VERBOSE(logger, "--------------------------------------------------");
        LOG_VERBOSE(logger, "SocketID: %u", GetSocketID());
        LOG_VERBOSE(logger, "MessageID: %u", GetMessageID());
        LOG_VERBOSE(logger, "QoS Level: %u", GetQoS());
        LOG_VERBOSE(logger, "Retain: %s", IsRetain() ? "true" : "false");
        LOG_VERBOSE(logger, "Topic: %s", GetTopicString());
        LOG_VERBOSE(logger, "Payload: %s", GetPayload());
        LOG_VERBOSE(logger, "--------------------------------------------------\n\n");
    #endif
    }

    Message::Message(Message&& obj) noexcept
        : mMacAddress(std::move(obj.mMacAddress))
        , mSocketID(std::move(obj.mSocketID))
        , mMessageID(std::move(obj.mMessageID))
        , mQoS(std::move(obj.mQoS))
        , mRetainFlag(std::move(obj.mRetainFlag))
        , mTopicCode(std::move(obj.mTopicCode))
        , mPayload(std::move(obj.mPayload))
        , mTopicString(std::move(obj.mTopicString))
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "Constructed by Move from %p to %p", &obj, this);
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "SocketID: %u", GetSocketID());
        LOG_DEBUG(logger, "MessageID: %u", GetMessageID());
        LOG_DEBUG(logger, "QoS Level: %u", GetQoS());
        LOG_DEBUG(logger, "Retain: %s", IsRetain() ? "true" : "false");
        LOG_DEBUG(logger, "Topic: %s", GetTopicString());
        LOG_DEBUG(logger, "Payload: %s", GetPayload());
        LOG_DEBUG(logger, "--------------------------------------------------\n\n");
    #endif
    }

    Message::~Message()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    socket_e Message::GetSocketID() const
    {
        return mSocketID;
    }

    uint16_t Message::GetMessageID() const
    {
        return mMessageID;
    }

    qos_e Message::GetQoS() const
    {
        return mQoS;
    }

    bool Message::IsRetain() const
    {
        return mRetainFlag;
    }

    topic_e Message::GetTopic() const
    {
        return mTopicCode;
    }

    const char* Message::GetTopicString() const
    {
        return mTopicString.c_str();
    }

    const char* Message::GetPayload() const
    {
        return mPayload.c_str();
    }
}}