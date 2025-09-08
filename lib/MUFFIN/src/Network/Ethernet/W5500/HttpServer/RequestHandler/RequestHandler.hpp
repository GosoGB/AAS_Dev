/**
 * @file RequestHandler.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-09-08
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>
#include <vector>

#include "../HttpServer.hpp"
#include "../Include/HttpMethod.hpp"

#include "Common/Assert.hpp"



namespace muffin { namespace w5500 {


    class RequestHandler
    {
    public:
        RequestHandler() = default;
        virtual ~RequestHandler() noexcept = default;

    public:
        virtual bool CanHandle(const http_method_e method, const std::string& requestURI)
        {
            (void) method;
            (void) requestURI;
            return false;
        }

        virtual bool CanUpload(const std::string& requestURI)
        {
            (void) requestURI;
            return false;
        }

        virtual bool Handle(HttpServer& server, const http_method_e requestMethod, const std::string& requestURI)
        {
            (void) server;
            (void) requestMethod;
            (void) requestURI;
            return false;
        }

        virtual void Upload(HttpServer& server, const std::string& requestURI, HTTPUpload& upload)
        {
            (void) server;
            (void) requestURI;
            (void) upload;
        }

        RequestHandler* GetNext()
        {
            return mNext;
        }

        void SetNext(RequestHandler* requestHandler)
        {
            mNext = requestHandler;
        }


        size_t GetPathArgSize() const
        {
            return mVectorPathArgs.size();
        }
        
        const std::string& GetPathArg(const uint32_t idx) const
        {
            ASSERT((idx < mVectorPathArgs.size()), "BAD ARGS: OUT OF RANGE");
            return mVectorPathArgs[idx];
        }

    protected:
        std::vector<std::string> mVectorPathArgs;
    private:
        RequestHandler* mNext = nullptr;
    };
}}