/**
 * @file FunctionHandler.hpp
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

#include "./RequestHandler.hpp"
#include "../Include/URI.hpp"



namespace muffin { namespace w5500 {


    class FunctionHandler : public RequestHandler
    {
    public:
        FunctionHandler(HttpServer::cbHandler fn, HttpServer::cbHandler ufn, const URI& uri, const http_method_e method)
            : mFunction(fn)
            , mUploadFunction(ufn)
            , mURI(uri.Clone())
            , mMethod(method)
        {
            mURI->InitPathArgs(&mVectorPathArgs);
        }

        ~FunctionHandler() noexcept override
        {
            delete mURI;
        }
        
        bool CanHandle(const http_method_e requestMethod, const std::string& requestURI) override
        {
            if (mMethod != HTTP_ANY_METHOD && mMethod != requestMethod)
            {
                return false;
            }
            return mURI->CanHandle(requestURI, mVectorPathArgs);
        }

        bool CanUpload(const std::string& requestURI) override
        {
            if (!mUploadFunction || !CanHandle(HTTP_POST, requestURI))
            {
                return false;
            }
            return true;
        }

        bool Handle(HttpServer& server, const http_method_e requestMethod, const std::string& requestURI) override
        {
            (void) server;
            if (!CanHandle(requestMethod, requestURI))
            {
                return false;
            }

            mFunction();
            return true;
        }

        void Upload(HttpServer& server, const std::string& requestURI, HTTPUpload& upload) override
        {
            (void) server;
            (void) upload;
            if (CanUpload(requestURI))
            {
                mUploadFunction();
            }
        }

    protected:
        HttpServer::cbHandler mFunction;
        HttpServer::cbHandler mUploadFunction;
        URI* mURI;
        http_method_e mMethod;
    };
}}