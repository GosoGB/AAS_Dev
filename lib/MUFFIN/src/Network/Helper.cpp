/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 모듈에서 공통적으로 사용할 수 있는 함수를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Helper.h"



namespace muffin { namespace network { namespace lte {

    const char* ConvertPdpToString(const pdp_ctx_e context)
    {
        switch (context)
        {
        case pdp_ctx_e::PDP_01:
            return "#01";
        case pdp_ctx_e::PDP_02:
            return "#02";
        case pdp_ctx_e::PDP_03:
            return "#03";
        case pdp_ctx_e::PDP_04:
            return "#04";
        case pdp_ctx_e::PDP_05:
            return "#05";
        case pdp_ctx_e::PDP_06:
            return "#06";
        case pdp_ctx_e::PDP_07:
            return "#07";
        case pdp_ctx_e::PDP_08:
            return "#08";
        case pdp_ctx_e::PDP_09:
            return "#09";
        case pdp_ctx_e::PDP_10:
            return "#10";
        case pdp_ctx_e::PDP_11:
            return "#11";
        case pdp_ctx_e::PDP_12:
            return "#12";
        case pdp_ctx_e::PDP_13:
            return "#13";
        case pdp_ctx_e::PDP_14:
            return "#14";
        case pdp_ctx_e::PDP_15:
            return "#15";
        case pdp_ctx_e::PDP_16:
            return "#16";
        default:
            ASSERT(false, "INVALID PDP CONTEXT: %u", static_cast<uint8_t>(context));
            return nullptr;
        }
    }

    const char* ConvertSslToString(const ssl_ctx_e context)
    {
        switch (context)
        {
        case ssl_ctx_e::SSL_0:
            return "#0";
        case ssl_ctx_e::SSL_1:
            return "#1";
        case ssl_ctx_e::SSL_2:
            return "#2";
        case ssl_ctx_e::SSL_3:
            return "#3";
        case ssl_ctx_e::SSL_4:
            return "#4";
        case ssl_ctx_e::SSL_5:
            return "#5";
        default:
            ASSERT(false, "INVALID SSL CONTEXT: %u", static_cast<uint8_t>(context));
            return nullptr;
        }
    }
}}}