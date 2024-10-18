/**
 * @file RequestParameter.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 요청 파라미터를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "RequestParameter.h"



namespace muffin { namespace http {

    RequestParameter::RequestParameter()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    RequestParameter::~RequestParameter()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    const char* RequestParameter::c_str() const
    {
        return ToString().c_str();
    }

    std::string RequestParameter::ToString() const
    {
        if (mMapParameters.size() == 0)
        {
            return "";
        }
        
        std::string param = "?";

        auto it = mMapParameters.begin();
        param += it->first + "=" + it->second;
        ++it;

        while (it != mMapParameters.end())
        {
            param += "&" + it->first + "=" + it->second;
            ++it;
        }
    #if defined(DEBUG)
        LOG_DEBUG(logger, "HTTP Parameter: %s", param.c_str());
    #endif
        return param;
    }

    void RequestParameter::AddParameter(const std::string& key, const std::string& value)
    {
        mMapParameters.emplace(key, value);
    }
}}