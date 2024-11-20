/**
 * @file MacAddress.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "MacAddress.h"



namespace muffin {

    MacAddress* MacAddress::CreateInstanceOrNULL() noexcept
    {
        if (mInstance == nullptr)
        {
            Status ret = readMacAddressesFromAllNIC();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO READ MAC ADDRESS: %s", ret.c_str());
                return mInstance;
            }

            mInstance = new(std::nothrow) MacAddress();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO ALLOCATE MEMORY FOR MAC ADDRESS");
                return mInstance;
            }
        }

        return mInstance;
    }

    MacAddress& MacAddress::GetInstance() noexcept
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }
    
    MacAddress::MacAddress()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    MacAddress::~MacAddress()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    const char* MacAddress::GetEthernet()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return mEthernet.c_str();
    }

    const char* MacAddress::GetWiFiClient()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return mWiFiClient.c_str();
    }

    const char* MacAddress::GetWiFiServer()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return mWiFiServer.c_str();
    }

    esp_err_t MacAddress::readMacAddress(const esp_mac_type_t type, std::string* mac)
    {
        ASSERT((mac != nullptr), "OUTPUT PARAMETER <mac> CANNOT BE A NULL POINTER");
        ASSERT((mac->empty() == true), "OUTPUT PARAMETER <mac> MUST BE EMPTY");

        uint8_t baseMAC[6] = { 0, 0, 0, 0, 0, 0 };
        const esp_err_t ret = esp_read_mac(baseMAC, type);
        if (ret != ESP_OK)
        {
            return ret;
        }

        char buffer[13] = {0};
        sprintf(buffer, "%02X%02X%02X%02X%02X%02X",
            baseMAC[0], baseMAC[1], baseMAC[2],
            baseMAC[3], baseMAC[4], baseMAC[5]
        );

        mac->append(buffer);
        return ret;
    }

    Status MacAddress::readMacAddressEthernet()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_ETH, &mEthernet);
        if (ret != ESP_OK)
        {
            LOG_ERROR(logger, "FAILED TO READ ETHERNET MAC: %s", esp_err_to_name(ret));
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressWiFiClient()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_STA, &mWiFiClient);
        if (ret != ESP_OK)
        {
            LOG_ERROR(logger, "FAILED TO READ Wi-Fi Client MAC: %s", esp_err_to_name(ret));
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressWiFiServer()
    {
        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_SOFTAP, &mWiFiServer);
        if (ret != ESP_OK)
        {
            LOG_ERROR(logger, "FAILED TO READ Wi-Fi Server MAC: %s", esp_err_to_name(ret));
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status MacAddress::readMacAddressesFromAllNIC()
    {
        Status::Code arrayStatusCode[INTERFACES_COUNT];
        arrayStatusCode[0] = readMacAddressEthernet().ToCode();
        arrayStatusCode[1] = readMacAddressWiFiClient().ToCode();
        arrayStatusCode[2] = readMacAddressWiFiServer().ToCode();
        
        for (uint8_t i = 0; i < INTERFACES_COUNT; ++i)
        {
            if (arrayStatusCode[i] != Status::Code::GOOD)
            {
                mEthernet.clear();
                mWiFiClient.clear();
                mWiFiServer.clear();

                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
        }

        return Status(Status::Code::GOOD);
    }


    MacAddress* MacAddress::mInstance = nullptr;
    std::string MacAddress::mEthernet;
    std::string MacAddress::mWiFiClient;
    std::string MacAddress::mWiFiServer;
}