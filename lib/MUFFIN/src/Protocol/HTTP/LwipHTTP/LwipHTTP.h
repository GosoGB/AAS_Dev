/**
 * @file LwipHTTP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LWIP TCP/IP stack 기반의 HTTP 클라이언트 클래스를 선언합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <bitset>
#include <WiFiClientSecure.h>

#include "Common/Status.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/HTTP/Include/RequestBody.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/HTTP/Include/RequestParameter.h"
#include "Protocol/HTTP/IHTTP.h"



namespace muffin { namespace http {

    class LwipHTTP : public IHTTP
    {
    public:
        LwipHTTP() {}
        virtual ~LwipHTTP() {}
    private:
        WiFiClient mClient;
        WiFiClientSecure mClientSecure;
    private:
        std::string mResponseData;
    public:
        Status Init();
        virtual Status GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status Retrieve(const size_t mutexHandle, std::string* response) override;
        virtual INetwork* RetrieveNIC() override;
    private:
        Status getHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status getHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status postHTTP(RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60);
        Status postHTTPS(RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60);
        Status postHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status postHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
    private:
        Status processResponseHeader(const uint16_t timeout = 60);
        std::string getHttpBody(const std::string& payload);
    };
}}