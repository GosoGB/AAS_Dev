/**
 * @file Core.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <esp_system.h>

#include "Common/Status.h"



namespace muffin {
    
    class Core
    {
    public:
        Core(Core const&) = delete;
        void operator=(Core const&) = delete;
        static Core& GetInstance();
    private:
        Core();
        virtual ~Core();
    private:
        static Core* mInstance;

    public:
        Status Init();
    private:
        esp_reset_reason_t mResetReason;
        std::string mMacAddressEthernet;
        std::string mMacAddressWiFiClient;
        std::string mMacAddressWiFiServer;
    };
}