/**
 * @file Constants.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 사용하는 상수를 정의합니다.
 * 
 * @date 2025-02-05
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
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
    constexpr const char* NVS_NAMESPACE_INIT = "init";

    /**
     * @brief SPIFFS 파티션 읽기/쓰기에 사용되는 상수 정의
     */
    constexpr const char* INIT_FILE_PATH         = "/init/config.csv";
    constexpr const char* JARVIS_PATH            = "/jarvis/config.json";
    constexpr const char* JARVIS_PATH_FETCHED    = "/jarvis/fetched.json";
    constexpr const char* OTA_REQUEST_PATH       = "/ota/request.json";
    constexpr const char* OTA_CHUNK_PATH_ESP32   = "/ota/chunk_esp32.csv";
    constexpr const char* OTA_CHUNK_PATH_MEGA    = "/ota/chunk_mega2560.csv";

    typedef enum class TaskName
        : uint8_t
    {
        MQTT_TASK            = 0,
        CYCLICALS_MSG_TASK   = 1,
        MODBUS_RTU_TASK      = 2,
        MODBUS_TCP_TASK      = 3,
        MORNITOR_ALARM_TASK  = 4,
        OPERATION_TIME_TASK  = 5,
        PRODUCTION_INFO_TASK = 6,
        CATM1_PROCESSOR_TASK = 7
    } task_name_e;

    typedef enum class ReconfigurationCode
        : uint8_t
    {
        NONE                        = 0, // 설정 변경 없음
        JARVIS_USER_CONFIG_CHANGE   = 1, // MFM 사용자에 의한 설정 변경
        JARVIS_USER_FACTORY_RESET   = 2, // MFM 사용자에 의한 공장 초기화
        FIRMWARE_DEFAULT_RECOVERY   = 3, // 설정 값 예외/실패로 인해 반복 리셋 시 펌웨어 기본값 복원
        CLI_USER_CONFIG_CHANGE      = 4  // CLI를 통해 사용자가 직접 설정 변경
    } reconfiguration_code_e;
}