/**
 * @file HttpServer.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-09-09
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "../EthernetClient.h"
#include "./Include/HttpMethod.hpp"
#include "./Include/URI.hpp"
// #include "./RequestHandler/RequestHandler.hpp"



namespace muffin { namespace w5500 {


    class HttpServer;


    typedef enum class HTTPUploadStatus
    {
        UPLOAD_FILE_START,
        UPLOAD_FILE_WRITE,
        UPLOAD_FILE_END,
        UPLOAD_FILE_ABORTED
    } http_upload_status_e;

    typedef enum class HTTPClientStatus
    {
        HC_NONE,
        HC_WAIT_READ,
        HC_WAIT_CLOSE
    } http_client_status_e;

    typedef enum class HTTPAuthMethod
    {
        BASIC_AUTH,
        DIGEST_AUTH
    } http_auth_method_e;

    typedef struct HTTPUpload 
    {
        HTTPUploadStatus Status;
        std::string FileName;
        std::string Name;
        std::string Type;
        size_t  TotalSize;    // file size
        size_t  CurrentSize;  // size of data currently in buf
        // uint8_t Buffer[HttpServer::UPLOAD_BUFFER_SIZE];
        uint8_t Buffer[1436];
    } http_upload_t;


    class HttpServer
    {
    public:
        HttpServer() = default;
        ~HttpServer() noexcept = default;
    public:
        void Begin();
        void Begin(const uint16_t port);
        void Close();
        void Stop();
    public:
        // void HandleClient();
        // bool Authenticate(const char* username, const char* password);
        // void RequestAuthentication(const http_auth_method_e mode = http_auth_method_e::BASIC_AUTH, const char* realm = NULL, const std::string& authFailMsg);
    public:
        typedef std::function<void(void)> cbHandler;
        // void OnEvent(const URI& uri, cbHandler fn);
        // void OnEvent(const URI& uri, const http_method_e method, cbHandler func);
        // void AddHandler(RequestHandler* handler);
        // void OnNotFound(cbHandler func);
    public:
        std::string GetURI() const noexcept { return mCurrentURI; }
        http_method_e GetMethod() const noexcept { return mCurrentMethod; }
        // EthernetClient GetClieint() const noexcept { return mCurrentClient; }
        std::string GetPathArg(const uint32_t idx) const;
        // std::string GetArgName(const uint32_t idx) const;
        // std::string GetArgValue(const std::string& argName) const;
        // std::string GetArgValue(const uint32_t idx) const;
        // bool HasArg(const std::string& argName);
    public:
        size_t GetContentLength() const { return mContentLength; }
    private:
        std::string mCurrentURI;
        http_method_e mCurrentMethod;
        EthernetClient mCurrentClient;
        size_t mContentLength;
    public:
        static const uint16_t DOWNLOAD_UNIT_SIZE = 1436;
        static const uint16_t UPLOAD_BUFFER_SIZE = 1436;
        static const uint16_t MAX_DATA_WAIT = 5000;
        static const uint16_t MAX_POST_WAIT = 5000;
        static const uint16_t MAX_SEND_WAIT = 5000;
        static const uint16_t MAX_CLOSE_WAIT = 2000;
        static const size_t CONTENT_LENGTH_UNKNOWN = ((size_t) -1);
        static const size_t CONTENT_LENGTH_NOT_SET = ((size_t) -2);
    };
}}