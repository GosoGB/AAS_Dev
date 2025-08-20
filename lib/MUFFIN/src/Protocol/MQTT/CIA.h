/**
 * @file CIA.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 브로커로부터 수신한 메시지를 집적하여 관리하는 클래스를 선언합니다.
 * @details Centralized Incoming-MQTT-messages Aggregator(CIA) 클래스는 네트워크
 *          인터페이스의 유형에 구애받지 않고 MQTT 브로커로부터 수신한 모든 메시지를
 *          한 곳에 집적해서 관리하기 위한 기능을 제공합니다.
 * 
 * @date 2025-01-22
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
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
        CIA();
        virtual ~CIA();
    public:
        uint8_t Count();
        Status Store(const Message& message, const uint32_t timeoutMillis = 1000);
        std::pair<Status, Message> Retrieve(const uint32_t timeoutMillis = 1000);
        std::pair<Status, Message> Peek(const uint32_t timeoutMillis = 1000);
    private:
        const uint8_t MAX_QUEUE_LENGTH = 10;
        const size_t MESSAGE_SIZE = sizeof(Message*);
        QueueHandle_t mQueueHandle = NULL;
    };


    extern CIA cia;
}}