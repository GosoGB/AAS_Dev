/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN에서 사용하는 커스텀 정보 모델 데이터 타입들을 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <sys/_stdint.h>

#include "Common/Status.h"



namespace muffin {

    typedef enum class McuTypeEnum : uint8_t
    {
        MCU_ESP32,
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        MCU_ATmega2560
    #endif
    } mcu_type_e;

    typedef struct InitConfigFileType
    {
        int8_t PanicResetCount;
        int8_t HasPendingJARVIS;
        int8_t HasPendingUpdate;
    } init_cfg_t;

    typedef struct FirmwareChunkInfoFileType
    {
        uint8_t Index;
        char Path[64];
        char CRC32[9];
        size_t Size;
    } ota_chunk_info_t;

    typedef muffin::Status (*CallbackUpdateInitConfig)(const muffin::init_cfg_t&);
}