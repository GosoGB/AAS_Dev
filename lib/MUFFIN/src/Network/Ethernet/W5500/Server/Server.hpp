/**
 * @file Server.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-09-09
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#pragma once

#include <memory>

#include "../EthernetClient.h"
#include "../Socket.h"
#include "../W5500.h"

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "Common/Sync/Mutex.hpp"




namespace muffin { namespace w5500 {


    class Server
    {
    public:
        Server(const uint16_t port = 80, uint8_t maxClient = 1)
            : mPort(port)
            , mMaxClients(maxClient)
            , mListening(false)
            , mNoDelay(false)
        {
            LOG_VERBOSE(logger, "Port: %d", mPort);
        }

        Server(const IPAddress& address, const uint16_t port = 80, uint8_t maxClient = 1)
            : mAddress(address)
            , mPort(port)
            , mMaxClients(maxClient)
            , mListening(false)
            , mNoDelay(false)
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
        // void End();
    public:
        // size_t Write(const uint8_t inputData[], const size_t length);
        // size_t Write(const uint8_t inputData);
        // using Print::write;
        EthernetClient Available();
        // EthernetClient Accept() { return Available(); }

    private:
        std::shared_ptr<Socket> mSocket;
        std::shared_ptr<Socket> mAcceptedSocket;
        IPAddress mAddress;
        uint16_t mPort;
        uint8_t mMaxClients;
        bool mListening;
        bool mNoDelay;
        Mutex mMutex;
    };
}}


#endif