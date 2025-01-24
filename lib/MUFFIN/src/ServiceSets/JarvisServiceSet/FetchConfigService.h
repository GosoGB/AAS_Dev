/**
 * @file FetchConfigService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MFM 서버로부터 최신 설정 정보를 가져오는 함수를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"

constexpr const char* DOWNLOADED_JARVIS_PATH = "/jarvis/config.tmp";



namespace muffin {

    Status FetchConfigService();
}
