/**
 * @file DownloadFirmwareService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어를 다운로드 하는 서비스를 선언합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <freertos/queue.h>

#include "Common/Status.h"
#include "Common/CRC32/CRC32.h"
#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    Status DownloadFirmwareService(ota::fw_info_t& info, CRC32& crc32, QueueHandle_t queue);
}