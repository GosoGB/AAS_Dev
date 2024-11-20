/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-14
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace http {

    http_rsc_e ConvertInt32ToRSC(const int32_t intRSC);
    const char* ConvertRscToString(const http_rsc_e rsc);
    std::string ConvertMethodToString(const rest_method_e method);
    const char* ConvertSchemeToString(const http_scheme_e scheme);
}}