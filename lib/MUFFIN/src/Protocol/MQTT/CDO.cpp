/**
 * @file CDO.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MQTT 브로커로부터 수신한 메시지를 집적하여 관리하는 클래스를 선언합니다.
 * 
 * @date 2025-01-22
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "CDO.h"
#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"




namespace muffin { namespace mqtt {

    CDO::CDO()
    {
        mQueueHandle = xQueueCreate(MAX_QUEUE_LENGTH, MESSAGE_SIZE);
        if (mQueueHandle == NULL)
        {
            std::cerr << "\n\n\033[31m" << "FAILED TO ALLOCATE MEMORY FOR MESSAGE QUEUE" << std::endl;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            std::abort();
        }
    }
    
    CDO::~CDO()
    {
        vQueueDelete(mQueueHandle);
    }

    uint8_t CDO::Count()
    {
        return static_cast<uint8_t>(uxQueueMessagesWaiting(mQueueHandle));
    }

    Status CDO::Store(const Message& message, const uint32_t timeoutMillis)
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
            delete movedMessage;
            return Status(Status::Code::BAD);
        }
    }

    std::pair<Status, Message> CDO::Retrieve(const uint32_t timeoutMillis)
    {
        ASSERT((Count() != 0), "NO STORED MESSAGE FOUND: CHECK IF THERE'S A MESSAGE BY CALLING \"COUNT\" FUNCTION");
        
        Message* pMessage;
        BaseType_t ret = xQueueReceive(mQueueHandle, &pMessage, static_cast<TickType_t>(timeoutMillis));

        if (ret == pdTRUE)
        {
            Message message = std::move(*pMessage);
            delete pMessage;
            return std::make_pair(Status(Status::Code::GOOD), message);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), Message());
        }
    }

    std::pair<Status, Message> CDO::Peek(const uint32_t timeoutMillis)
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
            return std::make_pair(Status(Status::Code::BAD), Message());
        }
    }


    CDO cdo;
}}