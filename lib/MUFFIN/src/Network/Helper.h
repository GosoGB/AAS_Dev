/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 모듈에서 공통적으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace network { namespace lte {

    const char* ConvertPdpToString(const pdp_ctx_e context);
    const char* ConvertSslToString(const ssl_ctx_e context);
}}}