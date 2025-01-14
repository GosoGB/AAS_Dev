/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 업데이트에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <string.h>
#include <sys/_stdint.h>

#include "Protocol/HTTP/Include/TypeDefinitions.h"



namespace muffin { namespace ota {

    typedef struct NewFirmwareInfoType
    {
        bool MCU_ESP32;
        bool MCU_MEGA2560;
    } new_fw_t;

    typedef struct UrlInfoType
    {
        char Host[64];
        uint16_t Port;
        http_scheme_e Scheme;
    } url_t;

    typedef struct FirmwareOtaInfoType
    {
        static size_t OtaID;
        uint32_t VersionCode;
        uint64_t FileTotalSize;
        char TotalChecksum[9];
        char SemanticVersion[16];  // 기존 "FirmwareVersion"
        uint8_t FileNumberArray[192];
        size_t FileSizeArray[192];
        char FilePathArray[192][64];
        char FileChecksumArray[192][9];
    } ota_info_t;
}}