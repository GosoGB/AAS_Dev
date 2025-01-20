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

    typedef enum class McuTypeEnum : uint8_t
    {
        MCU1  = 1,
        MCU2  = 2
    } mcu_e;

    typedef struct UrlInfoType
    {
        char Host[64];
        uint16_t Port;
        http_scheme_e Scheme;
    } url_t;

    typedef struct FirmwareInfoHeadType
    {
        static uint32_t ID;
        static url_t API;
        char SemanticVersion[16];
        uint32_t VersionCode;
        bool HasNewFirmware;
        mcu_e MCU;
    } fw_head_t;

    typedef struct FirmwareChunkInfoHeadType
    {
        uint8_t Count;
        uint8_t FlashingIDX;
        uint8_t DownloadIDX;
        uint8_t IndexArray[192];
        char PathArray[192][64];
    } chk_head_t;
    
    typedef struct FirmwareChecksumInfoType
    {
        char Total[9];
        char Array[192][9];
    } fw_cks_t;

    typedef struct FirmwareSizeInfoType
    {
        size_t Total;
        size_t Array[192];
    } fw_size_t;

    typedef struct FirmwareInfoType
    {
        fw_head_t Head;
        chk_head_t Chunk;
        fw_cks_t Checksum;
        fw_size_t Size;
    } fw_info_t;

    
    size_t fw_head_t::ID;
    url_t fw_head_t::API;
}}