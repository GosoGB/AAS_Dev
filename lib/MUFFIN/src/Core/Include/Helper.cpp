/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Core 모듈 전반에 걸쳐 공통으로 사용할 수 있는 함수를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 1.0.0
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
}