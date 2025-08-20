/**
 * @file RequestBody.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 바디 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-19
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "RequestBody.h"



namespace muffin { namespace http {
    
    RequestBody::RequestBody(const std::string& contentType)
        : mContentType(contentType)
    {
        ASSERT((contentType == "application/x-www-form-urlencoded"), "CURRENTY, ONLY \"application/x-www-form-urlencoded\" IS SUPPORTED");
    }
    
    RequestBody::~RequestBody()
    {
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