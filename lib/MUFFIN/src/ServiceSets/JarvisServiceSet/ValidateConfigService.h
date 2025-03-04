/**
 * @file ValidateConfigService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MFM 설정 정보의 유효성을 검증하는 서비스를 선언합니다.
 * 
 * @date 2025-01-28
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "DataFormat/JSON/JSON.h"



namespace muffin {

    Status ValidateConfigService(jarvis_struct_t* output);
}