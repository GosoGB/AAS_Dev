/**
 * @file Constants.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 사용하는 상수를 정의합니다.
 * 
 * @date 2025-01-13
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin {

    /**
     * @brief 일반적으로 사용하는 상수를 정의
     */
    constexpr uint8_t  MAX_RETRY_COUNT   = 5;
    constexpr uint16_t SECOND_IN_MILLIS  = 1000;
    constexpr uint16_t KILLOBYTE         = 1024;

    /**
     * @brief NVS 파티션 읽기/쓰기에 사용되는 상수를 정의
     */
    constexpr const char* NVS_NAMESPACE_INIT              = "init";
    constexpr const char* NVS_KEY_PANIC_RESET_COUNT       = "prc";
    constexpr const char* NVS_KEY_JARVIS_RESET_BY_CRASH   = "jvs_rst";
    constexpr const char* NVS_KEY_FLAG_JARVIS_REQUEST     = "jvs_req";

    /**
     * @brief SPIFFS 파티션 읽기/쓰기에 사용되는 상수를 정의
     */
    constexpr const char* FS_JARVIS_PATH       = "/jarvis/config.json";
    constexpr const char* FS_JARVIS_PATH_TMP   = "/jarvis/config.tmp";
}