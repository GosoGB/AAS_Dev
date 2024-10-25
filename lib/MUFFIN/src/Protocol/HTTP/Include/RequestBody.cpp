/**
 * @file RequestBody.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 바디 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-19
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "RequestBody.h"



namespace muffin { namespace http {
    
    RequestBody::RequestBody(const std::string& contentType)
        : mContentType(contentType)
    {
    #if defined(DEBUG)
        ASSERT((contentType == "application/x-www-form-urlencoded"),
            "CURRENTY, ONLY \"application/x-www-form-urlencoded\" IS SUPPORTED");
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }
    
    RequestBody::~RequestBody()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    std::string RequestBody::ToString() const
    {
        auto it = mMapProperty.begin();
        std::string body = it->first + "=" + it->second;
        ++it;

        while (it != mMapProperty.end())
        {
            body += "&" + it->first + "=" + it->second;
            ++it;
        }
    #if defined(DEBUG)
        LOG_DEBUG(logger, "HTTP Body: %s", body.c_str());
    #endif
        return body;
    }

    void RequestBody::AddProperty(const std::string& key, const std::string& value)
    {
        mMapProperty.emplace(key, value);
    }

    const char* RequestBody::GetContentType() const
    {
        return mContentType.c_str();
    }
}}