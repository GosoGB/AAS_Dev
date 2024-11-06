/**
 * @file CDO.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MQTT 브로커로부터 수신한 메시지를 집적하여 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "CDO.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"




namespace muffin { namespace mqtt {

    CDO* CDO::CreateInstanceOrNULL()
    {
        if (mInstance == nullptr)
        {
            mQueueHandle = xQueueCreate(MAX_QUEUE_LENGTH, MESSAGE_SIZE);
            if (mQueueHandle == NULL)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MESSAGE QUEUE");
                return mInstance;
            }

            mInstance = new(std::nothrow) CDO();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT CDO");
                return mInstance;
            }
        }

        return mInstance;
    }

    CDO& CDO::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    CDO::CDO()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    CDO::~CDO()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
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
        LOG_DEBUG(logger, " Stored Message size : %u", sizeof(Message));
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

    std::pair<Status, Message> CDO::Retrieve(const uint32_t timeoutMillis)
    {
        ASSERT((Count() != 0), "NO STORED MESSAGE FOUND: CHECK IF THERE'S A MESSAGE BY CALLING \"COUNT\" FUNCTION");
        
        Message* pMessage;
        BaseType_t ret = xQueueReceive(mQueueHandle, &pMessage, static_cast<TickType_t>(timeoutMillis));
        LOG_DEBUG(logger, "Retrieve message PAYLOAD: %s", pMessage->GetPayload());
        LOG_DEBUG(logger, "Retrieve message TOPIC: %s", pMessage->GetTopicString());

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

    std::pair<Status, Message> CDO::Peek(const uint32_t timeoutMillis)
    {
        ASSERT((Count() != 0), "NO STORED MESSAGE FOUND: CHECK IF THERE'S A MESSAGE BY CALLING \"COUNT\" FUNCTION");
    
        Message* pMessage;
        BaseType_t ret = xQueuePeek(mQueueHandle, &pMessage, static_cast<TickType_t>(timeoutMillis));

        LOG_DEBUG(logger, "Peek message PAYLOAD: %s", pMessage->GetPayload());
        LOG_DEBUG(logger, "Peek message TOPIC: %s", pMessage->GetTopicString());
      
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


    CDO* CDO::mInstance = nullptr;
    QueueHandle_t CDO::mQueueHandle = NULL;
}}