/**
 * @file FetchFirmwareInfo.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어 정보를 가져오는 서비스를 선언합니다.
 * 
 * @date 2025-01-16
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <string>

#include "Common/Status.h"
#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    Status FetchFirmwareInfo(const ota::fw_info_t& info, std::string* output);
}