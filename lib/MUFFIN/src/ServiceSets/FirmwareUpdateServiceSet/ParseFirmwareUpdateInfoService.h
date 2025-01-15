/**
 * @file ParseFirmwareUpdateInfoService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 업데이트 정보를 파싱하는 서비스를 선언합니다.
 * 
 * @date 2025-01-15
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"

#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

#if defined(MODLINK_L)
    Status ParseFirmwareUpdateInfoService(const char* payload, ota::fw_info_t* esp32);
#elif defined(MODLINK_T2)
    Status ParseFirmwareUpdateInfoService(const char* payload, ota::fw_info_t* esp32, ota::fw_info_t* mega2560);
#endif
}