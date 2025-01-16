/**
 * @file IHTTP.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 1.2.2
 * @date 2025-01-13
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
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