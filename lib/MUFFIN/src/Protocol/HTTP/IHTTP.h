/**
 * @file IHTTP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief HTTP 프로토콜에 대한 인터페이스를 선언합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Network/INetwork.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/HTTP/Include/RequestBody.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/HTTP/Include/RequestParameter.h"


namespace muffin { namespace http {

    class IHTTP
    {
    public:
        IHTTP() {}
        virtual ~IHTTP() {}
    public:
        virtual Status GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) = 0;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60) = 0 ;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) = 0;
        virtual Status Retrieve(const size_t mutexHandle, std::string* response) = 0;
        virtual INetwork* RetrieveNIC() = 0;
    };
}}


extern muffin::http::IHTTP* httpClient;