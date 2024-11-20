/**
 * @file RequestHeader.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 헤더 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-19
 * @version 1.0.0
 * 
 * @note 헤더 설정 시 유효성을 검증하는 절차가 필요하다는 생각으로 모든
 *       setter 함수가 Status를 반환하도록 시그니처를 작성했습니다. 단,
 *       현재는 시간 제약으로 인해 구현은 어렵습니다. 향후 버전 개발 시
 *       본 노트를 참조하면 좋겠습니다.
 * 
 * @todo 필요하다고 생각되거나 시간적인 여유가 있다면 setter 함수의 
 *       매개변수로 들어온 데이터의 유효성을 검증할 수 있는 절차를 
 *       구현해야 합니다.
 * 
 * @todo UserAgent 속성에 들어갈 MODLINK 모델 이름과 펌웨어 버전 정보는
 *       향후 정보모델 내에서 관리되는 값을 참조하도록 수정되어야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <bitset>
#include <string>

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace http {

    class RequestHeader
    {
    public:
        RequestHeader(const rest_method_e method, const http_scheme_e scheme, const std::string& host, const uint16_t port, const std::string& path, const std::string& userAgent);
        virtual ~RequestHeader();
    public:
        std::string ToString() const;
        std::string GetURL() const;
    public:
        Status SetAccept(const std::string& accept);
        Status SetAcceptEncoding(const std::string& encoding);
        Status SetCacheControl(const std::string& cacheControl);
        Status SetConnection(const std::string& connection);
        Status SetContentLength(const size_t contentLength);
        Status SetContentType(const std::string& contentType);
        Status SetHost(const std::string& host);
        Status SetPort(const uint16_t port);
        Status SetUserAgent(const std::string& userAgent);
        Status SetVersion(const std::string& version);
        Status UpdateParamter(const std::string& parameter);
    public:
        const std::string& GetAccept() const;
        const std::string& GetAcceptEncoding() const;
        const std::string& GetCacheControl() const;
        const std::string& GetConnection() const;
        size_t GetContentLength() const;
        const std::string& GetContentType() const;
        const std::string& GetHost() const;
        uint16_t GetPort() const;
        const std::string& GetUserAgent() const;
        const std::string& GetVersion() const;
    private:
        // bool isValidMediaType(const std::string& mediaType);
        // bool isValidQualityFactor(const std::string& qualityFactor);
    private:
        std::string mAccept;
        std::string mAcceptEncoding;
        std::string mCacheControl;
        std::string mConnection;
        size_t mContentLength;
        std::string mContentType;
        std::string mHost;
        rest_method_e mMethod;
        std::string mPath;
        uint16_t mPort;
        http_scheme_e mScheme;
        std::string mUserAgent;
        std::string mVersion;
    private:
        typedef enum class HeaderPropertyFlagEnum
            : uint8_t
        {
            ACCEPT            =  0,
            ACCEPT_ENCODING   =  1,
            CACHE_CONTROL     =  2,
            CONNECTION        =  3,
            CONTENT_LENGTH    =  4,
            CONTENT_TYPE      =  5,
            HOST              =  6,
            METHOD            =  7,
            PATH              =  8,
            PORT              =  9,
            SCHEME            = 10,
            USER_AGENT        = 11,
            VERSION           = 12
        } property_flag_e;
        std::bitset<13> mPropertyFlags;
    };
}}