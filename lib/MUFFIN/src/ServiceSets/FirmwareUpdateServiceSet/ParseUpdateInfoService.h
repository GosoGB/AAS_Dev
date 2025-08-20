/**
 * @file ParseUpdateInfoService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 업데이트 정보를 파싱하는 서비스를 선언합니다.
 * 
 * @date 2025-01-20
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"

#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    Status ParseUpdateInfoService(const char* payload, ota::fw_info_t* esp32, ota::fw_info_t* mega2560);
}