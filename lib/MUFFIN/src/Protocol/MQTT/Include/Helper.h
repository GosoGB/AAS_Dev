/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    const char* ConvertVersionToString(const version_e version);
    version_e ConvertUInt32ToVersion(const uint32_t integer);
}}