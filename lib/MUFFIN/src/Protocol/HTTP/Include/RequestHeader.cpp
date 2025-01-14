/**
 * @file RequestHeader.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 헤더 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-19
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex>
#include <sstream>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "RequestHeader.h"
#include "Helper.h"



namespace muffin { namespace http {

    RequestHeader::RequestHeader(const rest_method_e method, const http_scheme_e scheme, const std::string& host, const uint16_t port, const std::string& path, const std::string& userAgent)
        : mAccept("*/*")
        , mAcceptEncoding("gzip, deflate, br")
        , mCacheControl("no-cache")
        , mConnection("keep-alive")
        , mContentLength(0)
        , mHost(host)
        , mMethod(method)
        , mPath(path)
        , mPort(port)
        , mScheme(scheme)
        , mUserAgent(userAgent)
        , mVersion("HTTP/1.1")
    {
        mPropertyFlags.set();
        mPropertyFlags.reset(static_cast<uint8_t>(property_flag_e::CONTENT_LENGTH));
        mPropertyFlags.reset(static_cast<uint8_t>(property_flag_e::CONTENT_TYPE));
    }

    RequestHeader::~RequestHeader()
    {
    }

    std::string RequestHeader::ToString() const
    {
        std::string header = ConvertMethodToString(mMethod) + " " + mPath + " " + mVersion + "\r\n";
        header += "Host: " + mHost + ":" + std::to_string(mPort)  + "\r\n";
        header += "User-Agent: " + mUserAgent + "\r\n";
        header += "Accept: " + mAccept + "\r\n";
        header += "AcceptEncoding: " + mAcceptEncoding + "\r\n";
        header += "Cache-Control: " + mCacheControl + "\r\n";
        header += "Connection: " + mConnection + "\r\n";

        if (mPropertyFlags.test(static_cast<uint8_t>(property_flag_e::CONTENT_LENGTH)))
        {
            header += "Content-Length: " + std::to_string(mContentLength) + "\r\n";
        }
        
        if (mPropertyFlags.test(static_cast<uint8_t>(property_flag_e::CONTENT_TYPE)))
        {
            header += "Content-Type: " + mContentType + "\r\n";
        }

        header += "\r\n";
        return header;
    }
    
    std::string RequestHeader::GetURL() const
    {
        return ConvertSchemeToString(mScheme) + mHost + ":" + std::to_string(mPort) + mPath;
    }

    Status RequestHeader::SetAccept(const std::string& accept)
    {
        mAccept = accept;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetAcceptEncoding(const std::string& encoding)
    {
        mAcceptEncoding = encoding;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetCacheControl(const std::string& cacheControl)
    {
        mCacheControl = cacheControl;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetConnection(const std::string& connection)
    {
        mConnection = connection;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetContentLength(const size_t contentLength)
    {
        mContentLength = contentLength;
        mPropertyFlags.set(static_cast<uint8_t>(property_flag_e::CONTENT_LENGTH));
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetContentType(const std::string& contentType)
    {
        mContentType = contentType;
        mPropertyFlags.set(static_cast<uint8_t>(property_flag_e::CONTENT_TYPE));
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetHost(const std::string& host)
    {
        mHost = host;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetPort(const uint16_t port)
    {
        ASSERT((port != 0), "PORT NUMBER MUST BE GREATER THAN 0");

        if (port != 0)
        {
            mPort = port;
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status RequestHeader::SetUserAgent(const std::string& userAgent)
    {
        mUserAgent = userAgent;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::SetVersion(const std::string& version)
    {
        mVersion = version;
        return Status(Status::Code::GOOD);
    }

    Status RequestHeader::UpdateParamter(const std::string& parameter)
    {
        mPath += parameter;
        LOG_INFO(logger,"mPath : %s",mPath.c_str());
        return Status(Status::Code::GOOD);
    }

    const std::string& RequestHeader::GetAccept() const
    {
        return mAccept;
    }

    const std::string& RequestHeader::GetAcceptEncoding() const
    {
        return mAcceptEncoding;
    }

    const std::string& RequestHeader::GetCacheControl() const
    {
        return mCacheControl;
    }

    const std::string& RequestHeader::GetConnection() const
    {
        return mConnection;
    }

    size_t RequestHeader::GetContentLength() const
    {
        return mContentLength;
    }

    const std::string& RequestHeader::GetContentType() const
    {
        return  mContentType;
    }

    const std::string& RequestHeader::GetHost() const
    {
        return mHost;
    }

    uint16_t RequestHeader::GetPort() const
    {
        return mPort;
    }

    const std::string& RequestHeader::GetUserAgent() const
    {
        return mUserAgent;
    }

    const std::string& RequestHeader::GetVersion() const
    {
        return mVersion;
    }

    const http_scheme_e& RequestHeader::GetSchem() const
    {
        return mScheme;
    }

}}