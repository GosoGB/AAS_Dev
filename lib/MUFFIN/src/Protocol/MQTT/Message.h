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

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <string>
#include <sys/_stdint.h>

#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    class Message
    {
    public:
        Message(topic_e topic, const std::string& payload, const socket_e socketID = socket_e::SOCKET_0, const uint16_t messageID = 0, const qos_e qos = qos_e::QoS_0, const bool isRetain = false);
        Message(const Message& obj);
        Message(Message&& obj) noexcept;
        virtual ~Message();
    public:
        socket_e GetSocketID() const;
        uint16_t GetMessageID() const;
        qos_e GetQoS() const;
        bool IsRetain() const;
        topic_e GetTopic() const;
        const char* GetTopicString() const;
        const char* GetPayload() const;
        static topic_e Convert2Topic(const std::string& topic);
        static topic_e Convert2Topic(const char* topic);
        const char* Convert2TopicString() const;
        static const char* Convert2TopicString(const topic_e topic);
    private:
        const socket_e mSocketID;
        const uint16_t mMessageID;
        const MqttQoSEnum mQoS;
        const bool mRetainFlag;
        const topic_e mTopic;
        const std::string mPayload;
        static const char* mArrayValidTopics[];
    };

    constexpr uint8_t QUEUE_LENGTH_RECEIVED_MESSAGE = 5;
    constexpr size_t QUEUE_SIZE_MQTT_MESSAGE = sizeof(Message);
    // QueueHandle_t QUEUE_MQTT_MESSAGE;
}}