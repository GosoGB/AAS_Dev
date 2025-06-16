/**
 * @file MacAddress.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-01-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <string.h>
#include "Common/Assert.h"
#include "MacAddress.h"



namespace muffin {
    
    MacAddress::MacAddress()
    {
        Status ret = readMacAddressesFromAllNIC();
        if (ret != Status::Code::GOOD)
        {
            std::cerr << "\n\n\033[31mFATAL ERROR: FAILED TO READ MAC ADDRESS" << std::endl;
            std::abort();
        }
    }

    Status MacAddress::SetMacAddress(char mac[])
    {
        strncpy(mEthernet, mac, sizeof(mEthernet) - 1);
        mEthernet[sizeof(mEthernet) - 1] = '\0';

        return Status(Status::Code::GOOD);
    }

    const char* MacAddress::GetEthernet()
    {
        return mEthernet;
    }

    const char* MacAddress::GetWiFiClient()
    {
        return mWiFiClient;
    }

    const char* MacAddress::GetWiFiServer()
    {
        return mWiFiServer;
    }

    esp_err_t MacAddress::readMacAddress(const esp_mac_type_t type, char output[])
    {
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        uint8_t baseMAC[6] = { 0, 0, 0, 0, 0, 0 };
        const esp_err_t ret = esp_read_mac(baseMAC, type);
        if (ret != ESP_OK)
        {
            return ret;
        }

        sprintf(output, "%02X%02X%02X%02X%02X%02X",
            baseMAC[0], baseMAC[1], baseMAC[2],
            baseMAC[3], baseMAC[4], baseMAC[5]
        );

        return ret;
    }

    Status MacAddress::readMacAddressEthernet()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_ETH, mEthernet);
        if (ret != ESP_OK)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressWiFiClient()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_STA, mWiFiClient);
        if (ret != ESP_OK)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressWiFiServer()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_SOFTAP, mWiFiServer);
        if (ret != ESP_OK)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressesFromAllNIC()
    {
        Status::Code arrayStatusCode[NIC_COUNT];
        arrayStatusCode[0] = readMacAddressEthernet().ToCode();
        arrayStatusCode[1] = readMacAddressWiFiClient().ToCode();
        arrayStatusCode[2] = readMacAddressWiFiServer().ToCode();
        
        for (uint8_t i = 0; i < NIC_COUNT; ++i)
        {
            if (arrayStatusCode[i] != Status::Code::GOOD)
            {
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
        }

        return Status(Status::Code::GOOD);
    }


    MacAddress macAddress;
}