/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Core 모듈 전반에 걸쳐 공통으로 사용할 수 있는 함수를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <esp_mac.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Helper.h"



namespace muffin {

    void printResetReason(const esp_reset_reason_t reason)
    {
        switch (reason)
        {
        case ESP_RST_UNKNOWN:
            LOG_INFO(muffin::logger, "Reset reason: Cannot be determined");
            break;
        case ESP_RST_POWERON:
            LOG_INFO(muffin::logger, "Reset reason: Due to power-on event");
            break;
        case ESP_RST_EXT:
            LOG_INFO(muffin::logger, "Reset reason: By External pin");
            break;
        case ESP_RST_SW:
            LOG_INFO(muffin::logger, "Reset reason: By software reset");
            break;
        case ESP_RST_PANIC:
            LOG_INFO(muffin::logger, "Reset reason: Due to exception or panic");
            break;
        case ESP_RST_INT_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to interrupt watchdog");
            break;
        case ESP_RST_TASK_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to task watchdog");
            break;
        case ESP_RST_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to other watchdog");
            break;
        case ESP_RST_DEEPSLEEP:
            LOG_INFO(muffin::logger, "Reset reason: By deep sleep mode");
            break;
        case ESP_RST_BROWNOUT:
            LOG_INFO(muffin::logger, "Reset reason: By Brownout");
            break;
        case ESP_RST_SDIO:
            LOG_INFO(muffin::logger, "Reset reason: Due to error with SDIO interface");
            break;
        default:
            ASSERT(false, "UNDEFINED RESET REASON CODE: %u", reason);
            break;
        }
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

    Status readMacAddressEthernet(std::string* mac)
    {
        ASSERT((mac != nullptr), "OUTPUT PARAMETER <mac> CANNOT BE A NULL POINTER");
        ASSERT((mac->empty() == true), "OUTPUT PARAMETER <mac> MUST BE EMPTY");

        const esp_err_t ret = readMacAddress(ESP_MAC_ETH, mac);
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

    Status readMacAddressWiFiClient(std::string* mac)
    {
        ASSERT((mac != nullptr), "OUTPUT PARAMETER <mac> CANNOT BE A NULL POINTER");
        ASSERT((mac->empty() == true), "OUTPUT PARAMETER <mac> MUST BE EMPTY");

        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_STA, mac);
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

    Status readMacAddressWiFiServer(std::string* mac)
    {
        ASSERT((mac != nullptr), "OUTPUT PARAMETER <mac> CANNOT BE A NULL POINTER");
        ASSERT((mac->empty() == true), "OUTPUT PARAMETER <mac> MUST BE EMPTY");

        const esp_err_t ret = readMacAddress(ESP_MAC_WIFI_SOFTAP, mac);
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
}