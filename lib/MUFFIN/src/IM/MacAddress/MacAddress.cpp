/**
 * @file MacAddress.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 식별자로 사용되는 MAC 주소를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "MacAddress.h"



namespace muffin {

    MacAddress* MacAddress::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) MacAddress();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MAC ADDRESS");
                return mInstance;
            }
        }
        /*개체 생성 완료*/
        
        if (mInstance->mHasMacAddresses == false)
        {
            Status::Code arrayReturn[3];
            arrayReturn[0] = mInstance->readMacAddressEthernet().ToCode();
            arrayReturn[1] = mInstance->readMacAddressWiFiClient().ToCode();
            arrayReturn[2] = mInstance->readMacAddressWiFiServer().ToCode();
            
            for (uint8_t i = 0; i < 3; ++i)
            {            
                if (arrayReturn[i] != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO READ MAC ADDRESS: %s", Status(arrayReturn[i]).c_str());

                    mInstance->mEthernet.clear();
                    mInstance->mWiFiClient.clear();
                    mInstance->mWiFiServer.clear();

                    return nullptr;
                }
            }

            mInstance->mHasMacAddresses == true;
        }
        /*Ethernet, Wi-Fi 인터페이스의 MAC 주소 읽기 성공*/

        return mInstance;
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
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    const std::string& MacAddress::GetEthernet() const
    {
        ASSERT((mInstance == nullptr), "CALL FUNCTION \"GetInstance()\" BEFORE CALLING GETTER");
        return mEthernet;
    }

    const std::string& MacAddress::GetWiFiClient() const
    {
        ASSERT((mInstance == nullptr), "CALL FUNCTION \"GetInstance()\" BEFORE CALLING GETTER");
        return mWiFiClient;
    }

    const std::string& MacAddress::GetWiFiServer() const
    {
        ASSERT((mInstance == nullptr), "CALL FUNCTION \"GetInstance()\" BEFORE CALLING GETTER");
        return mWiFiServer;
    }

    esp_err_t readMacAddress(const esp_mac_type_t type, std::string* mac)
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


    MacAddress* MacAddress::mInstance = nullptr;
}