/**
 * @file CatHTTP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 HTTP 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <bitset>

#include "Common/Status.h"
#include "Network/CatM1/CatM1.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/HTTP/Include/RequestBody.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/HTTP/Include/RequestParameter.h"



namespace muffin { namespace http {

    class CatHTTP
    {
    public:
        CatHTTP(CatHTTP const&) = delete;
        void operator=(CatHTTP const&) = delete;
        static CatHTTP* GetInstanceOrNULL(CatM1& catM1);
        static CatHTTP& GetInstance();
    private:
        CatHTTP(CatM1& catM1);
        virtual ~CatHTTP();
    private:
        static CatHTTP* mInstance;

    public:
        Status Init(const network::lte::pdp_ctx_e pdp, const network::lte::ssl_ctx_e ssl, const bool customRequestHeader = true, const bool outputResponse = false);
        Status GET(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout = 60);
        Status POST(RequestHeader& header, const RequestBody& body, const uint16_t timeout = 60);
        Status Retrieve(std::string* response);
        void SetSinkToCatFS(const bool save2CatFS);
        Status IsInitialized() const;
    private:
        Status setPdpContext(const network::lte::pdp_ctx_e pdp);
        Status setSslContext(const network::lte::ssl_ctx_e ssl);
        Status setCustomRequestHeader(const bool enable);
        Status setResponseHeaderOutput(const bool turnOff);
    private:
        // Status checkPdpContext();
        // Status checkSslContext();
        // Status checkCustomRequestHeader();
        // Status checkResponseHeaderOutput();
    private:
        Status sendResponse2CatFS();
        Status setRequestURL(const std::string& url, const uint16_t timeout = 60);
    private:
        Status readUntilCONNECT(const uint32_t timeoutMillis, std::string* rxd);
        Status readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd);
        Status readUntilRSC(const uint32_t timeoutMillis, std::string* rxd);
        Status processCmeErrorCode(const std::string& rxd);
        Status convertErrorCode(const uint16_t errorCode);
    private:
        CatM1& mCatM1;
        network::lte::pdp_ctx_e mContextPDP;
        network::lte::ssl_ctx_e mContextSSL;
        bool mEnableCustomRequestHeader;
        bool mEnableResponseHeaderOutput;
        bool mSetSinkToCatFS;
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