/**
 * @file CIA.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 브로커로부터 수신한 메시지를 집적하여 관리하는 클래스를 선언합니다.
 * 
 * @date 2025-01-22
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "CIA.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace mqtt {

    CIA::CIA()
    {
        mQueueHandle = xQueueCreate(MAX_QUEUE_LENGTH, MESSAGE_SIZE);
        if (mQueueHandle == NULL)
        {
            std::cerr << "\n\n\033[31m" << "FAILED TO ALLOCATE MEMORY FOR MESSAGE QUEUE" << std::endl;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            std::abort();
        }
    }
    
    CIA::~CIA()
    {
        vQueueDelete(mQueueHandle);
    }

    uint8_t CIA::Count()
    {
        return static_cast<uint8_t>(uxQueueMessagesWaiting(mQueueHandle));
    }

    Status CIA::Store(const Message& message, const uint32_t timeoutMillis)
    {
        ASSERT((Count() < MAX_QUEUE_LENGTH), "NO SPACE TO STORE: CHECK THE NUMBER OF STORED MESSAGE BY CALLING \"COUNT\" FUNCTION");
        
        Message* movedMessage = new(std::nothrow) Message(message);
        ASSERT((movedMessage != nullptr), "MOVE OPERATION ON MQTT MESSAGE CANNOT FAIL");

        BaseType_t ret = xQueueSend(mQueueHandle, (void*)&movedMessage, static_cast<TickType_t>(timeoutMillis));
        if (ret == pdTRUE)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD);
        }
    }

    std::pair<Status, Message> CIA::Retrieve(const uint32_t timeoutMillis)
    {
        ASSERT((Count() != 0), "NO STORED MESSAGE FOUND: CHECK IF THERE'S A MESSAGE BY CALLING \"COUNT\" FUNCTION");
        
        Message* pMessage;
        BaseType_t ret = xQueueReceive(mQueueHandle, &pMessage, static_cast<TickType_t>(timeoutMillis));

        if (ret == pdTRUE)
        {
            Message message = std::move(*pMessage);
            delete pMessage;
            pMessage = nullptr;
            return std::make_pair(Status(Status::Code::GOOD), message);
        }
        else
        {
            delete pMessage;
            pMessage = nullptr;
            return std::make_pair(Status(Status::Code::BAD), Message());
        }
    }

    std::pair<Status, Message> CIA::Peek(const uint32_t timeoutMillis)
    {
        ASSERT((Count() != 0), "NO STORED MESSAGE FOUND: CHECK IF THERE'S A MESSAGE BY CALLING \"COUNT\" FUNCTION");
        
        Message* pMessage;
        BaseType_t ret = xQueuePeek(mQueueHandle, &pMessage, static_cast<TickType_t>(timeoutMillis));

        if (ret == pdTRUE)
        {
            Message message = *pMessage;
            return std::make_pair(Status(Status::Code::GOOD), message);
        }
        else
        {
            delete pMessage;
            pMessage = nullptr;
            return std::make_pair(Status(Status::Code::BAD), Message());
        }
    }


    CIA cia;
}}