/**
 * @file FindChunkInfoService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPIFFS 파티션으로부터 특정 OTA 청크 파일에 대한 정보를 찾아 반환하는 서비스를 선언합니다.
 * 
 * @date 2025-02-06
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "IM/Custom/TypeDefinitions.h"
#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    Status ReadIndexFromFirstLine(const ota::mcu_e mcuType, uint8_t* index);
    Status FindChunkInfoService(const ota::mcu_e mcuType, const uint8_t idx, ota_chunk_info_t* info);
}