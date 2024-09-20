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

    const char* Message::mArrayValidTopics[] = {
        "scautr/modlink/status/network/will", 
        "write/param", 
        "read/param"
    };

    Message::Message(topic_e topic, const std::string& payload, const socket_e socketID, const uint16_t messageID, const qos_e qos, const bool isRetain)
        : mSocketID(socketID)
        , mMessageID(messageID)
        , mQoS(qos)
        , mRetainFlag(isRetain)
        , mTopic(topic)
        , mPayload(payload)
    {
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
        : mSocketID(obj.mSocketID)
        , mMessageID(obj.mMessageID)
        , mQoS(obj.mQoS)
        , mRetainFlag(obj.mRetainFlag)
        , mTopic(obj.mTopic)
        , mPayload(obj.mPayload)
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "--------------------------------------------------");
        LOG_DEBUG(logger, "Constructed by Copy from %p to %p", &obj, this);
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

    Message::Message(Message&& obj) noexcept
        : mSocketID(std::move(obj.mSocketID))
        , mMessageID(std::move(obj.mMessageID))
        , mQoS(std::move(obj.mQoS))
        , mRetainFlag(std::move(obj.mRetainFlag))
        , mTopic(std::move(obj.mTopic))
        , mPayload(std::move(obj.mPayload))
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
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
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
        return mTopic;
    }

    const char* Message::GetTopicString() const
    {
        return mArrayValidTopics[static_cast<uint8_t>(mTopic)];
    }

    const char* Message::GetPayload() const
    {
        return mPayload.c_str();
    }

    topic_e Message::Convert2Topic(const std::string& topic)
    {
        return Convert2Topic(topic.c_str());
    }

    topic_e Message::Convert2Topic(const char* topic)
    {
        ASSERT(
            (
                strcmp(topic, mArrayValidTopics[0]) == 0 || 
                strcmp(topic, mArrayValidTopics[1]) == 0 || 
                strcmp(topic, mArrayValidTopics[2]) == 0
            ),
            "INVALID TOPIC: %s", topic
        );

        if (strcmp(topic, mArrayValidTopics[0]) == 0)
        {
            return topic_e::LAST_WILL_AND_TESTAMENT;
        }
        else if (strcmp(topic, mArrayValidTopics[1]) == 0)
        {
            return topic_e::JARVIS_WRITE_PARAMETER;
        }
        else
        {
            return topic_e::JARVIS_READ_PARAMETER;
        }
    }

    const char* Message::Convert2TopicString() const
    {
        return mArrayValidTopics[static_cast<uint8_t>(mTopic)];
    }

    const char* Message::Convert2TopicString(const topic_e topic)
    {
        return mArrayValidTopics[static_cast<uint8_t>(topic)];
    }
}}