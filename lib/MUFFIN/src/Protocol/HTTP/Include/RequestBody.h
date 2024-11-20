/**
 * @file RequestBody.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 프로토콜 바디 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-19
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <string>

#include "Common/Status.h"



namespace muffin { namespace http {

    class RequestBody
    {
    public:
        explicit RequestBody(const std::string& contentType);
        ~RequestBody();
    public:
        std::string ToString() const;
    public:
        void AddProperty(const std::string& key, const std::string& value);
        const char* GetContentType() const;
    private:
        std::string mContentType;
        std::map<std::string, std::string> mMapProperty;
    };
}}