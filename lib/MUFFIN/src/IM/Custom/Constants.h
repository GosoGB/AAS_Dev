/**
 * @file Constants.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 사용하는 상수를 정의합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin {

    /**
     * @brief 일반적으로 사용하는 상수를 정의
     */
    constexpr uint8_t   ONE_SECOND       = 1;
    constexpr uint8_t   MINUTE_IN_SEC    = 60 * ONE_SECOND;
    constexpr uint16_t  HOUR_IN_SEC      = 60 * MINUTE_IN_SEC;
    constexpr uint32_t  DAY_IN_SEC       = 24 * HOUR_IN_SEC;

    constexpr uint8_t  MAX_RETRY_COUNT   = 5;
    constexpr uint16_t SECOND_IN_MILLIS  = 1000;
    
    constexpr uint16_t KILLOBYTE         = 1024;

    constexpr uint32_t KHz = 1000;
    constexpr uint32_t MHz = 1000 * KHz;
    
    /**
     * @brief NVS 파티션 읽기/쓰기에 사용되는 상수를 정의
     */
    constexpr const char* NVS_NAMESPACE_INIT = "init";

    /**
     * @todo Ver.1.3 미만 펌웨어가 없다면 아래의 상수는 삭제할 예정임
     */
    constexpr const char* DEPRECATED_INIT_FILE_PATH         = "/init/config.csv";
    constexpr const char* DEPRECATED_JARVIS_PATH            = "/jarvis/config.json";
    constexpr const char* DEPRECATED_JARVIS_PATH_FETCHED    = "/jarvis/fetched.json";
    constexpr const char* DEPRECATED_OTA_REQUEST_PATH       = "/ota/request.json";
    constexpr const char* DEPRECATED_OTA_CHUNK_PATH_ESP32   = "/ota/chunk_esp32.csv";
    constexpr const char* DEPRECATED_OTA_CHUNK_PATH_MEGA    = "/ota/chunk_mega2560.csv";
    constexpr const char* DEPRECATED_LWIP_HTTP_PATH         = "/http/response";
    constexpr const char* DEPRECATED_SPEAR_LINK1_PATH       = "/spear/link1/config.json";
    constexpr const char* DEPRECATED_SPEAR_LINK2_PATH       = "/spear/link2/config.json";
    constexpr const char* DEPRECATED_SPEAR_PRTCL_PATH       = "/spear/protocol/config.json";

    /**
     * @brief SPIFFS 파티션 읽기/쓰기에 사용되는 상수 정의
     */
    constexpr const char* INIT_FILE_PATH         = "/init_config.csv";
    constexpr const char* JARVIS_PATH            = "/jarvis_config.json";
    constexpr const char* JARVIS_PATH_FETCHED    = "/jarvis_fetched.json";
    constexpr const char* SERVICE_URL_PATH       = "/service_url.json";
    constexpr const char* OTA_REQUEST_PATH       = "/ota_request.json";
    constexpr const char* OTA_CHUNK_PATH_ESP32   = "/ota_chunk_esp32.csv";
    constexpr const char* OTA_CHUNK_PATH_MEGA    = "/ota_chunk_mega2560.csv";
    constexpr const char* LWIP_HTTP_PATH         = "/http_response";
    

    typedef enum class TaskName
        : uint8_t
    {
        MQTT_TASK            = 0,
        PUBLISH_MSG_TASK   = 1,
        MODBUS_RTU_TASK      = 2,
        MODBUS_TCP_TASK      = 3,
        MORNITOR_ALARM_TASK  = 4,
        OPERATION_TIME_TASK  = 5,
        PRODUCTION_INFO_TASK = 6,
        CATM1_PROCESSOR_TASK = 7,
        CATM1_MONITORING_TASK = 8,
        MELSEC_TASK = 9,
        ETHERNET_IP_TASK = 10
    } task_name_e;

    typedef enum class ReconfigurationCode : uint8_t
    {
        NONE                        = 0, // 설정 변경 없음
        JARVIS_USER_CONFIG_CHANGE   = 1, // MFM 사용자에 의한 설정 변경
        JARVIS_USER_FACTORY_RESET   = 2, // MFM 사용자에 의한 공장 초기화
        FIRMWARE_DEFAULT_RECOVERY   = 3, // 설정 값 예외/실패로 인해 반복 리셋 시 펌웨어 기본값 복원
        CLI_USER_CONFIG_CHANGE      = 4  // CLI를 통해 사용자가 직접 설정 변경
    } reconfiguration_code_e;
}