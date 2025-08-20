/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-14
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <errno.h>

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Helper.h"



namespace muffin { namespace http {

    http_rsc_e ConvertInt32ToRSC(const int32_t intRSC)
    {
        switch (intRSC)
        {
        case 100:
            return http_rsc_e::CONTINUE;
        case 101:
            return http_rsc_e::SWITCHING_PROTOCOLS;
        case 102:
            return http_rsc_e::PROCESSING;
        case 200:
            return http_rsc_e::OK;
        case 201:
            return http_rsc_e::CREATED;
        case 202:
            return http_rsc_e::ACCEPTED;
        case 203:
            return http_rsc_e::NON_AUTHORITATIVE_INFORMATION;
        case 204:
            return http_rsc_e::NO_CONTENT;
        case 205:
            return http_rsc_e::RESET_CONTENT;
        case 206:
            return http_rsc_e::PARTIAL_CONTENT;
        case 207:
            return http_rsc_e::MULTI_STATUS;
        case 208:
            return http_rsc_e::ALREADY_REPORTED;
        case 226:
            return http_rsc_e::IM_USED;
        case 300:
            return http_rsc_e::MULTIPLE_CHOICES;
        case 301:
            return http_rsc_e::MOVED_PERMANENTLY;
        case 302:
            return http_rsc_e::FOUND;
        case 303:
            return http_rsc_e::SEE_OTHER;
        case 304:
            return http_rsc_e::NOT_MODIFIED;
        case 305:
            return http_rsc_e::USE_PROXY;
        case 307:
            return http_rsc_e::TEMPORARY_REDIRECT;
        case 308:
            return http_rsc_e::PERMANENT_REDIRECT;
        case 400:
            return http_rsc_e::BAD_REQUEST;
        case 401:
            return http_rsc_e::UNAUTHORIZED;
        case 402:
            return http_rsc_e::PAYMENT_REQUIRED;
        case 403:
            return http_rsc_e::FORBIDDEN;
        case 404:
            return http_rsc_e::NOT_FOUND;
        case 405:
            return http_rsc_e::METHOD_NOT_ALLOWED;
        case 406:
            return http_rsc_e::NOT_ACCEPTABLE;
        case 407:
            return http_rsc_e::PROXY_AUTHENTICATION_REQUIRED;
        case 408:
            return http_rsc_e::REQUEST_TIMEOUT;
        case 409:
            return http_rsc_e::CONFLICT;
        case 410:
            return http_rsc_e::GONE;
        case 411:
            return http_rsc_e::LENGTH_REQUIRED;
        case 412:
            return http_rsc_e::PRECONDITION_FAILED;
        case 413:
            return http_rsc_e::PAYLOAD_TOO_LARGE;
        case 414:
            return http_rsc_e::URI_TOO_LONG;
        case 415:
            return http_rsc_e::UNSUPPORTED_MEDIA_TYPE;
        case 416:
            return http_rsc_e::RANGE_NOT_SATISFIABLE;
        case 417:
            return http_rsc_e::EXPECTATION_FAILED;
        case 421:
            return http_rsc_e::MISDIRECTED_REQUEST;
        case 422:
            return http_rsc_e::UNPROCESSABLE_ENTITY;
        case 423:
            return http_rsc_e::LOCKED;
        case 424:
            return http_rsc_e::FAILED_DEPENDENCY;
        case 426:
            return http_rsc_e::UPGRADE_REQUIRED;
        case 428:
            return http_rsc_e::PRECONDITION_REQUIRED;
        case 429:
            return http_rsc_e::TOO_MANY_REQUESTS;
        case 431:
            return http_rsc_e::REQUEST_HEADER_FIELDS_TOO_LARGE;
        case 500:
            return http_rsc_e::INTERNAL_SERVER_ERRROR;
        case 501:
            return http_rsc_e::NOT_IMPLEMENTED;
        case 502:
            return http_rsc_e::BAD_GATEWAY;
        case 503:
            return http_rsc_e::SERVICE_UNAVAILABLE;
        case 504:
            return http_rsc_e::GATEWAY_TIMEOUT;
        case 505:
            return http_rsc_e::VERSION_NOT_SUPPORTED;
        case 506:
            return http_rsc_e::VARIANT_ALSO_NEGOTIATES;
        case 507:
            return http_rsc_e::INSUFFICIENT_STORAGE;
        case 508:
            return http_rsc_e::LOOP_DETECTED;
        case 510:
            return http_rsc_e::NOT_EXTENDED;
        case 511:
            return http_rsc_e::AUTHENTICATION_REQUIRED;
        default:
            LOG_ERROR(logger, "UNDEFINED RSC: %d", intRSC);
            return http_rsc_e::UNDEFINED_RSC;
        }
    }

    const char* ConvertRscToString(const http_rsc_e rsc)
    {
        switch (rsc)
        {
        case http_rsc_e::CONTINUE:
            return "CONTINUE";
        case http_rsc_e::SWITCHING_PROTOCOLS:
            return "SWITCHING PROTOCOLS";
        case http_rsc_e::PROCESSING:
            return "PROCESSING";
        case http_rsc_e::OK:
            return "OK";
        case http_rsc_e::CREATED:
            return "CREATED";
        case http_rsc_e::ACCEPTED:
            return "ACCEPTED";
        case http_rsc_e::NON_AUTHORITATIVE_INFORMATION:
            return "NON AUTHORITATIVE INFORMATION";
        case http_rsc_e::NO_CONTENT:
            return "NO CONTENT";
        case http_rsc_e::RESET_CONTENT:
            return "RESET CONTENT";
        case http_rsc_e::PARTIAL_CONTENT:
            return "PARTIAL CONTENT";
        case http_rsc_e::MULTI_STATUS:
            return "MULTI STATUS";
        case http_rsc_e::ALREADY_REPORTED:
            return "ALREADY REPORTED";
        case http_rsc_e::IM_USED:
            return "IM USED";
        case http_rsc_e::MULTIPLE_CHOICES:
            return "MULTIPLE CHOICES";
        case http_rsc_e::MOVED_PERMANENTLY:
            return "MOVED PERMANENTLY";
        case http_rsc_e::FOUND:
            return "FOUND";
        case http_rsc_e::SEE_OTHER:
            return "SEE OTHER";
        case http_rsc_e::NOT_MODIFIED:
            return "NOT MODIFIED";
        case http_rsc_e::USE_PROXY:
            return "USE PROXY";
        case http_rsc_e::TEMPORARY_REDIRECT:
            return "TEMPORARY REDIRECT";
        case http_rsc_e::PERMANENT_REDIRECT:
            return "PERMANENT REDIRECT";
        case http_rsc_e::BAD_REQUEST:
            return "BAD REQUEST";
        case http_rsc_e::UNAUTHORIZED:
            return "UNAUTHORIZED";
        case http_rsc_e::PAYMENT_REQUIRED:
            return "PAYMENT REQUIRED";
        case http_rsc_e::FORBIDDEN:
            return "FORBIDDEN";
        case http_rsc_e::NOT_FOUND:
            return "NOT FOUND";
        case http_rsc_e::METHOD_NOT_ALLOWED:
            return "METHOD NOT ALLOWED";
        case http_rsc_e::NOT_ACCEPTABLE:
            return "NOT ACCEPTABLE";
        case http_rsc_e::PROXY_AUTHENTICATION_REQUIRED:
            return "PROXY AUTHENTICATION REQUIRED";
        case http_rsc_e::REQUEST_TIMEOUT:
            return "REQUEST TIMEOUT";
        case http_rsc_e::CONFLICT:
            return "CONFLICT";
        case http_rsc_e::GONE:
            return "GONE";
        case http_rsc_e::LENGTH_REQUIRED:
            return "LENGTH REQUIRED";
        case http_rsc_e::PRECONDITION_FAILED:
            return "PRECONDITION FAILED";
        case http_rsc_e::PAYLOAD_TOO_LARGE:
            return "PAYLOAD TOO LARGE";
        case http_rsc_e::URI_TOO_LONG:
            return "URI TOO LONG";
        case http_rsc_e::UNSUPPORTED_MEDIA_TYPE:
            return "UNSUPPORTED MEDIA TYPE";
        case http_rsc_e::RANGE_NOT_SATISFIABLE:
            return "RANGE NOT SATISFIABLE";
        case http_rsc_e::EXPECTATION_FAILED:
            return "EXPECTATION FAILED";
        case http_rsc_e::MISDIRECTED_REQUEST:
            return "MISDIRECTED REQUEST";
        case http_rsc_e::UNPROCESSABLE_ENTITY:
            return "UNPROCESSABLE ENTITY";
        case http_rsc_e::LOCKED:
            return "LOCKED";
        case http_rsc_e::FAILED_DEPENDENCY:
            return "FAILED DEPENDENCY";
        case http_rsc_e::UPGRADE_REQUIRED:
            return "UPGRADE REQUIRED";
        case http_rsc_e::PRECONDITION_REQUIRED:
            return "PRECONDITION REQUIRED";
        case http_rsc_e::TOO_MANY_REQUESTS:
            return "TOO MANY REQUESTS";
        case http_rsc_e::REQUEST_HEADER_FIELDS_TOO_LARGE:
            return "REQUEST HEADER FIELDS TOO LARGE";
        case http_rsc_e::INTERNAL_SERVER_ERRROR:
            return "INTERNAL SERVER ERRROR";
        case http_rsc_e::NOT_IMPLEMENTED:
            return "NOT IMPLEMENTED";
        case http_rsc_e::BAD_GATEWAY:
            return "BAD GATEWAY";
        case http_rsc_e::SERVICE_UNAVAILABLE:
            return "SERVICE UNAVAILABLE";
        case http_rsc_e::GATEWAY_TIMEOUT:
            return "GATEWAY TIMEOUT";
        case http_rsc_e::VERSION_NOT_SUPPORTED:
            return "VERSION NOT SUPPORTED";
        case http_rsc_e::VARIANT_ALSO_NEGOTIATES:
            return "VARIANT ALSO NEGOTIATES";
        case http_rsc_e::INSUFFICIENT_STORAGE:
            return "INSUFFICIENT STORAGE";
        case http_rsc_e::LOOP_DETECTED:
            return "LOOP DETECTED";
        case http_rsc_e::NOT_EXTENDED:
            return "NOT EXTENDED";
        case http_rsc_e::AUTHENTICATION_REQUIRED:
            return "AUTHENTICATION REQUIRED";
        default:
            return nullptr;
        }
    }

    std::string ConvertMethodToString(const rest_method_e method)
    {
        switch (method)
        {
        case rest_method_e::GET:
            return "GET";
        case rest_method_e::POST:
            return "POST";
        case rest_method_e::PUT:
            return "PUT";
        case rest_method_e::PATCH:
            return "PATCH";
        case rest_method_e::DELETE:
            return "DELETE";
        case rest_method_e::OPTIONS:
            return "OPTIONS";
        case rest_method_e::HEAD:
            return "HEAD";
        default:
            return nullptr;
        }
    }

    const char* ConvertSchemeToString(const http_scheme_e scheme)
    {
        switch (scheme)
        {
        case http_scheme_e::HTTP:
            return "http://";
        case http_scheme_e::HTTPS:
            return "https://";
        default:
            return nullptr;
        }
    }
}}