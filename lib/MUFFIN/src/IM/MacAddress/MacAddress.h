/**
 * @file MacAddress.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
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
        static MacAddress* CreateInstanceOrNULL() noexcept;
        static MacAddress& GetInstance() noexcept;
    private:
        MacAddress();
        virtual ~MacAddress();
    private:
        static MacAddress* mInstance;

    public:
        static const char* GetEthernet();
        static const char* GetWiFiClient();
        static const char* GetWiFiServer();
    private:
        static esp_err_t readMacAddress(const esp_mac_type_t type, std::string* mac);
        static Status readMacAddressEthernet();
        static Status readMacAddressWiFiClient();
        static Status readMacAddressWiFiServer();
        static Status readMacAddressesFromAllNIC();
    private:
        static std::string mEthernet;
        static std::string mWiFiClient;
        static std::string mWiFiServer;
        static constexpr uint8_t INTERFACES_COUNT = 3;
    };
}