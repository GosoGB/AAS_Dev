/**
 * @file CatHTTP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 HTTP 프로토콜 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <bitset>

#include "Common/Status.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/HTTP/Include/RequestBody.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/HTTP/Include/RequestParameter.h"
#include "Protocol/HTTP/IHTTP.h"



namespace muffin { namespace http {

    class CatHTTP : public IHTTP
    {
    public:
        CatHTTP();
        virtual ~CatHTTP() override {}
    public:
        Status Init(const size_t mutex, const network::lte::pdp_ctx_e pdp, const network::lte::ssl_ctx_e ssl, const bool customRequestHeader = true, const bool outputResponse = false);
        virtual Status GET(const size_t mutex, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutex, RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60) override;
        virtual Status POST(const size_t mutex, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60) override;
        virtual Status Retrieve(const size_t mutex, std::string* response) override;
        virtual Status Retrieve(const size_t mutex, const size_t length, uint8_t response[]) override;
        void SetSinkToCatFS(const bool save2CatFS, const std::string catFsPath);
        Status IsInitialized() const;
    public:
        void OnEventReset();
    private:
        Status setPdpContext(const size_t mutex, const network::lte::pdp_ctx_e pdp);
        Status setSslContext(const size_t mutex, const network::lte::ssl_ctx_e ssl);
        Status setCustomRequestHeader(const size_t mutex, const bool enable);
        Status setResponseHeaderOutput(const size_t mutex, const bool turnOff);
    private:
        // Status checkPdpContext();
        // Status checkSslContext();
        // Status checkCustomRequestHeader();
        // Status checkResponseHeaderOutput();
    private:
        Status sendResponse2CatFS(const size_t mutex);
        Status setRequestURL(const size_t mutex, const std::string& url, const uint16_t timeout = 60);
    private:
        Status readUntilCONNECT(const uint32_t timeoutMillis, std::string* rxd);
        Status readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd);
        Status readUntilOKorERROR(const uint32_t timeoutMillis, const size_t length, uint8_t response[]);
        Status readUntilRSC(const uint32_t timeoutMillis, std::string* rxd);
        Status processCmeErrorCode(const std::string& rxd);
        Status convertErrorCode(const uint16_t errorCode);
    private:
        network::lte::pdp_ctx_e mContextPDP;
        network::lte::ssl_ctx_e mContextSSL;
        bool mEnableCustomRequestHeader;
        bool mEnableResponseHeaderOutput;
        bool mSetSinkToCatFS;
        std::string mCatFsPath;
    private:
        typedef enum CatHttpInitializationEnum
            : uint8_t
        {
            INITIALIZED_PDP   = 0, // Set if PDP context is initialized, reset otherwise
            INITIALIZED_SSL   = 1, // Set if SSL context is initialized, reset otherwise
            INITIALIZED_REQ   = 2, // Set if request header is initialized, reset otherwise
            INITIALIZED_RES   = 3, // Set if response output is initialized, reset otherwise
            INITIALIZED_ALL   = 4, // Set if initialization succeded, reset otherwise
        } init_flag_e;
        std::bitset<5> mInitFlags;

        typedef enum CatHttpStateEnum
            : int8_t
        {
            INIT_FAILED      = -1,
            CONSTRUCTED      =  0,
            INITIALIZED      =  1,
        } state_e;
        state_e mState;
    };
}}