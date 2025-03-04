/**
 * @file MacAddress.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 선언합니다.
 * 
 * @date 2025-01-13
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"



namespace muffin {

    class MacAddress
    {
    public:
        MacAddress();
        virtual ~MacAddress() {}

    public:
        const char* GetEthernet();
        const char* GetWiFiClient();
        const char* GetWiFiServer();
    private:
        esp_err_t readMacAddress(const esp_mac_type_t type, char output[]);
        Status readMacAddressEthernet();
        Status readMacAddressWiFiClient();
        Status readMacAddressWiFiServer();
        Status readMacAddressesFromAllNIC();
    private:
        char mEthernet[13]       = {'\0'};
        char mWiFiClient[13]     = {'\0'};
        char mWiFiServer[13]     = {'\0'};
        const uint8_t NIC_COUNT  = 3;
    };


    extern MacAddress macAddress;
}