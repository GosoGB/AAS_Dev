/**
 * @file IMQTT.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MQTT 프로토콜에 대한 인터페이스를 선언합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Network/INetwork.h"
#include "Protocol/MQTT/Include/Message.h"



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
        virtual INetwork* RetrieveNIC() = 0;
    };


    extern IMQTT* client;
}}