/**
 * @file FindChunkInfoService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPIFFS 파티션으로부터 특정 OTA 청크 파일에 대한 정보를 찾아 반환하는 서비스를 선언합니다.
 * 
 * @date 2025-02-05
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "IM/Custom/TypeDefinitions.h"



namespace muffin {

    Status FindChunkInfoService(const uint8_t idx, ota_chunk_info_t* info);
}