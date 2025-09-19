/**
 * @file Server.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-09-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#pragma once

#include <memory>
#include <vector>

#include "../EthernetClient.h"
#include "../Socket.h"
#include "../W5500.h"

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "Common/Sync/Mutex.hpp"




namespace muffin { namespace w5500 {


    namespace __internal {

        typedef enum ClientStatus
        {
            NO_CLIENT,
            WAIT_READ,
            WAIT_CLOSE
        } client_status_e;
    }

    
    class Server
    {
    public:
        Server(const uint16_t port = 80)
            : mClientStatus(__internal::client_status_e::NO_CLIENT)
            , mClientTimer(0)
            , mContentLength(0)
            , mPort(port)
            , mListening(false)
            , mHasClient(false)
        {
            LOG_VERBOSE(logger, "Port: %d", mPort);
        }

        Server(const IPAddress& address, const uint16_t port = 80)
            : mClientStatus(__internal::client_status_e::NO_CLIENT)
            , mClientTimer(0)
            , mContentLength(0)
            , mAddress(address)
            , mPort(port)
            , mListening(false)
            , mHasClient(false)
        {
            LOG_VERBOSE(logger, "IP: %s, Port: %d", mAddress.toString().c_str(), mPort);
        }

        ~Server() noexcept
        {
            // End();
        }

    public:
        void Begin(const uint16_t port = 0);
        void Begin(const uint16_t port, bool isReusable);
        // void SetNoDelay();
        // bool GetNoDelay() const;
        // bool HasClient() const;
        void Close();
        void Stop();
    public:
        // size_t Write(const uint8_t inputData[], const size_t length);
        // size_t Write(const uint8_t inputData);
        // using Print::write;
        bool HasClient();
        uint16_t Available();

    private:
        void collectHeaders(const char* headerKeys[], const size_t count);
    public: // 삭제할 것
        void handleClient();
    private:
        typedef struct RequestArgument
        {
            std::string Key;
            std::string value;
        } req_arg_t;
        std::vector<req_arg_t> mCurrentHeaders;
        int mHeaderKeyCount;

    private:
        __internal::client_status_e mClientStatus;
        uint32_t mClientTimer;
        size_t mContentLength;

    private:
        std::shared_ptr<Socket> mSocket;
        std::shared_ptr<EthernetClient> mClient;
        IPAddress mAddress;
        uint16_t mPort;
        bool mListening;
        bool mHasClient;
        Mutex mMutex;
    };
}}


#endif