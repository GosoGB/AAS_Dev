/**
 * @file CIA.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 브로커로부터 수신한 메시지를 집적하여 관리하는 클래스를 선언합니다.
 * @details Centralized Incoming-MQTT-messages Aggregator(CIA) 클래스는 네트워크
 *          인터페이스의 유형에 구애받지 않고 MQTT 브로커로부터 수신한 모든 메시지를
 *          한 곳에 집적해서 관리하기 위한 기능을 제공합니다.
 * 
 * @date 2024-10-17
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Common/Status.h"
#include "Include/Message.h"



namespace muffin { namespace mqtt {

    class CIA
    {
    public:
        CIA(CIA const&) = delete;
        void operator=(CIA const&) = delete;
        static CIA* GetInstanceOrNULL();
        static CIA& GetInstance();
    private:
        CIA();
        virtual ~CIA();
    private:
        static CIA* mInstance;

    public:
        static uint8_t Count();
        static Status Store(const Message& message, const uint32_t timeoutMillis = 1000);
        static std::pair<Status, Message> Retrieve(const uint32_t timeoutMillis = 1000);
        static std::pair<Status, Message> Peek(const uint32_t timeoutMillis = 1000);
    private:
        static constexpr uint8_t MAX_QUEUE_LENGTH = 10;
        static constexpr size_t MESSAGE_SIZE = sizeof(Message*);
        static QueueHandle_t mQueueHandle;
    };
}}