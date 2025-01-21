/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Core 모듈 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2025-01-21
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Logger/Logger.h"
#include "Helper.h"



namespace muffin {

    void LogResetReason(const esp_reset_reason_t resetReason)
    {
        switch (resetReason)
        {
        case ESP_RST_UNKNOWN:
            LOG_INFO(logger, "Reset reason: Cannot be determined");
            return;

        case ESP_RST_POWERON:
            LOG_INFO(logger, "Reset reason: Due to power-on event");
            return;

        case ESP_RST_EXT:
            LOG_INFO(logger, "Reset reason: By External pin");
            return;

        case ESP_RST_SW:
            LOG_INFO(logger, "Reset reason: By software reset");
            return;

        case ESP_RST_PANIC:
            LOG_INFO(logger, "Reset reason: Due to exception or panic");
            return;

        case ESP_RST_INT_WDT:
            LOG_INFO(logger, "Reset reason: Due to interrupt watchdog");
            return;

        case ESP_RST_TASK_WDT:
            LOG_INFO(logger, "Reset reason: Due to task watchdog");
            return;

        case ESP_RST_WDT:
            LOG_INFO(logger, "Reset reason: Due to other watchdog");
            return;

        case ESP_RST_DEEPSLEEP:
            LOG_INFO(logger, "Reset reason: By deep sleep mode");
            return;

        case ESP_RST_BROWNOUT:
            LOG_INFO(logger, "Reset reason: By Brownout");
            return;

        case ESP_RST_SDIO:
            LOG_INFO(logger, "Reset reason: Due to error with SDIO interface");
            return;

        default:
            ASSERT(false, "UNDEFINED RESET REASON CODE: %u", esp_reset_reason());
            return;
        }
    }
}