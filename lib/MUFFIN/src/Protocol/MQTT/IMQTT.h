/**
 * @file IMQTT.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 1.2.2
 * @date 2025-01-13
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 */




#pragma once

#include <vector>
#include "Common/Status.h"
#include "include/Message.h"



namespace muffin { namespace mqtt {

    class IMQTT
    {
    public:
        IMQTT() {}
        virtual ~IMQTT() {}
    public:
        virtual Status Connect(const size_t mutexHandle) = 0;
        virtual Status Disconnect(const size_t mutexHandle) = 0;
        virtual Status IsConnected() = 0;
        virtual Status Subscribe(const size_t mutexHandle, const std::vector<Message>& messages) = 0;
        virtual Status Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages) = 0;
        virtual Status Publish(const size_t mutexHandle, const Message& message) = 0;
    };
}}