/**
 * @file MacAddress.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @todo 향후 필요하면 블루투스의 MAC 주소도 추가해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>

#include "Common/Status.h"



namespace muffin {

    class MacAddress
    {
    public:
        MacAddress(const MacAddress&) = delete;
        void operator=(const MacAddress&) = delete;
        static MacAddress* GetInstance();
    private:
        MacAddress();
        virtual ~MacAddress();
    private:
        static MacAddress* mInstance;

    public:
        const std::string& GetEthernet() const;
        const std::string& GetWiFiClient() const;
        const std::string& GetWiFiServer() const;
    private:
        Status readMacAddressEthernet();
        Status readMacAddressWiFiClient();
        Status readMacAddressWiFiServer();
    private:
        bool mHasMacAddresses = false;
        std::string mEthernet;
        std::string mWiFiClient;
        std::string mWiFiServer;
    };
}