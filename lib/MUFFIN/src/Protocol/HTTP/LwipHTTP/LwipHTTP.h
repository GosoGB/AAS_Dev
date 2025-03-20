/**
 * @file LwipHTTP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LWIP TCP/IP stack 기반의 HTTP 클라이언트 클래스를 선언합니다.
 * 
 * @date 2025-01-21
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include "Common/Status.h"
#include "Common/DataStructure/bitset.h"
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
    public:
        Status Init();
        virtual Status GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status Retrieve(const size_t mutexHandle, std::string* response) override;
        virtual Status Retrieve(const size_t mutexHandle, const size_t length, uint8_t output[]) override;
        int32_t RetrieveContentLength() const;
    private:
        Status getHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status getHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status postHTTP(RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60);
        Status postHTTPS(RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60);
        Status postHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status postHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
    private:
        /**
         * @brief response header 정보를 추출합니다.
         * 
         * @param timeout 응답을 기다리는 최대 시간 (단위:초)
         * @param rsc response status code
         * @param contentLength response body 길이로 헤더에 속성이 없는 경우 -1을 반환
         * @return Status 
         */
        Status processResponseHeader(const uint16_t timeout);
        Status saveResponseBody();
    private:
        WiFiClient mClient;
        WiFiClientSecure mClientSecure;
    private:
        typedef enum class StatusFlagEnum : uint8_t
        {
            HTTP    = 1,
            HTTPS   = 2,
            RSC_OK  = 3,
            OCTET   = 4,
            TOP     = 5
        } flag_e;
        bitset<static_cast<uint8_t>(flag_e::TOP)> mFlags;
    private:
        uint16_t mRSC = 0;
        size_t mFilePosition = 0;
        int32_t mContentLength = 0;
    };


    extern LwipHTTP* lwipHTTP;
}}