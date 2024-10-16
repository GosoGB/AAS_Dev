/**
 * @file RequestParameter.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 요청 파라미터를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <string>

#include "Common/Status.h"



namespace muffin { namespace http {

    class RequestParameter
    {
    public:
        RequestParameter();
        ~RequestParameter();
    public:
        const char* c_str() const;
        std::string ToString() const;
    public:
        void AddParameter(const std::string& key, const std::string& value);
    private:
        std::map<std::string, std::string> mMapParameters;
    };    
}}