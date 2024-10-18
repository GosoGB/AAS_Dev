/**
 * @file Message.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 메시지 클래스를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    class Message
    {
    public:
        Message();
        Message(const topic_e topic, const std::string& payload, const socket_e socketID = socket_e::SOCKET_0, const uint16_t messageID = 0, const qos_e qos = qos_e::QoS_0, const bool isRetain = false);
        Message(const Message& obj);
        Message(Message&& obj) noexcept;
        virtual ~Message();
    public:
        void SetSocketID(const socket_e socketID);
        void SetMessageID(const uint16_t messageID);
        void SetQoS(const qos_e qosLevel);
        void SetRetain(const bool isRetain);
        void SetTopic(const topic_e topicCode);
        void SetPayload(const std::string& payload);
    public:
        socket_e GetSocketID() const;
        uint16_t GetMessageID() const;
        qos_e GetQoS() const;
        bool IsRetain() const;
        topic_e GetTopicCode() const;
        const char* GetTopicString() const;
        const char* GetPayload() const;
    private:
        bool mIsSocketIdSet;
        bool mIsMessageIdSet;
        bool mIsQosSet;
        bool mIsRetainSet;
        bool mIsTopicSet;
        bool mIsPayloadSet;
    private:
        socket_e mSocketID;
        uint16_t mMessageID;
        qos_e mQoS;
        bool mRetainFlag;
        topic_e mTopicCode;
        std::string mPayload;
    };
}}