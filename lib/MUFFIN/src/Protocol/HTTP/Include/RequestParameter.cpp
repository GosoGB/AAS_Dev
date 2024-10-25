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
#include "Core/Include/Helper.h"
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

    /**
     * @todo emplace 호출 시 발생 가능한 예외를 처리하도록 수정해야 합니다.
     * @todo Add 함수 호출자에게 처리 결과를 반환하도록 수정해야 합니다.
     */
    void RequestParameter::Add(const std::string& key, const std::string& value)
    {
        mMapParameters.emplace(key, value);
    }
}}