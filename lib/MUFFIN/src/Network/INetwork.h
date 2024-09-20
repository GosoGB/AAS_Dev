/**
 * @file INetwork.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 모듈이 공통으로 따르는 소프트웨어 인터페이스를 선언합니다.
 * 
 * @date 2024-09-04
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <IPAddress.h>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin {

    class INetwork
    {
    public:
        INetwork() {}
        virtual ~INetwork() {}
    public:
        virtual Status Init() = 0;
        virtual Status Config(jarvis::config::Base* config) = 0;
        virtual Status Connect() = 0;
        virtual Status Disconnect() = 0;
        virtual Status Reconnect() = 0;
        virtual bool IsConnected() const = 0;
        virtual IPAddress GetIPv4() const = 0;
    };
}