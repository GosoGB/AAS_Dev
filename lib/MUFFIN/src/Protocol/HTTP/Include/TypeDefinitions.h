/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-09-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin {

    typedef enum class HttpResponseStatusCodeEnum
        : int16_t
    {
        UNDEFINED_RSC                    =  -1,
        CONTINUE                         = 100,
        SWITCHING_PROTOCOLS              = 101,
        PROCESSING                       = 102,
        OK                               = 200,
        CREATED                          = 201,
        ACCEPTED                         = 202,
        NON_AUTHORITATIVE_INFORMATION    = 203,
        NO_CONTENT                       = 204,
        RESET_CONTENT                    = 205,
        PARTIAL_CONTENT                  = 206,
        MULTI_STATUS                     = 207,
        ALREADY_REPORTED                 = 208,
        IM_USED                          = 226,
        MULTIPLE_CHOICES                 = 300,
        MOVED_PERMANENTLY                = 301,
        FOUND                            = 302,
        SEE_OTHER                        = 303,
        NOT_MODIFIED                     = 304,
        USE_PROXY                        = 305,
        TEMPORARY_REDIRECT               = 307,
        PERMANENT_REDIRECT               = 308,
        BAD_REQUEST                      = 400,
        UNAUTHORIZED                     = 401,
        PAYMENT_REQUIRED                 = 402,
        FORBIDDEN                        = 403,
        NOT_FOUND                        = 404,
        METHOD_NOT_ALLOWED               = 405,
        NOT_ACCEPTABLE                   = 406,
        PROXY_AUTHENTICATION_REQUIRED    = 407,
        REQUEST_TIMEOUT                  = 408,
        CONFLICT                         = 409,
        GONE                             = 410,
        LENGTH_REQUIRED                  = 411,
        PRECONDITION_FAILED              = 412,
        PAYLOAD_TOO_LARGE                = 413,
        URI_TOO_LONG                     = 414,
        UNSUPPORTED_MEDIA_TYPE           = 415,
        RANGE_NOT_SATISFIABLE            = 416,
        EXPECTATION_FAILED               = 417,
        MISDIRECTED_REQUEST              = 421,
        UNPROCESSABLE_ENTITY             = 422,
        LOCKED                           = 423,
        FAILED_DEPENDENCY                = 424,
        UPGRADE_REQUIRED                 = 426,
        PRECONDITION_REQUIRED            = 428,
        TOO_MANY_REQUESTS                = 429,
        REQUEST_HEADER_FIELDS_TOO_LARGE  = 431,
        INTERNAL_SERVER_ERRROR           = 500,
        NOT_IMPLEMENTED                  = 501,
        BAD_GATEWAY                      = 502,
        SERVICE_UNAVAILABLE              = 503,
        GATEWAY_TIMEOUT                  = 504,
        VERSION_NOT_SUPPORTED            = 505,
        VARIANT_ALSO_NEGOTIATES          = 506,
        INSUFFICIENT_STORAGE             = 507,
        LOOP_DETECTED                    = 508,
        NOT_EXTENDED                     = 510,
        AUTHENTICATION_REQUIRED          = 511
    } http_rsc_e;

    typedef enum class HttpRestApiMethodEnum
        : uint8_t
    {
        GET     = 0,
        POST    = 1,
        PUT     = 2,
        PATCH   = 3,
        DELETE  = 4,
        OPTIONS = 5,
        HEAD    = 6
    } rest_method_e;

    typedef enum class HttpProtocolSchemeEnum
        : uint8_t
    {
        HTTP  = 0,
        HTTPS = 1
    } http_scheme_e;
}