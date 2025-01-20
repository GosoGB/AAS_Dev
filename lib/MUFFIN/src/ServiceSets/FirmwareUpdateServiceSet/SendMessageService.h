/**
 * @file SendMessageService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 상태를 발행하는 서비스를 선언합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    Status PublishFirmwareStatusMessageService();
    Status PostDownloadResult(const ota::fw_info_t& info, const char* result);
    Status PostUpdateResult(const ota::fw_info_t& info, const char* result);
}