/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace jarvis {

    uint8_t ConvertToUInt8(const char* str);
    const char* ConverKeyToString(const cfg_key_e key);
    std::pair<Status, cfg_key_e> ConvertToConfigKey(const prtcl_ver_e version, const char* str);
}}