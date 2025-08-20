/**
 * @file EthernetClient.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-19
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#pragma once

#include <memory>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "Common/Sync/Mutex.hpp"
#include "Socket.h"
#include "W5500.h"



namespace muffin { namespace w5500 {


    class EthernetClientRxBuffer;


    class EthernetClient : public Client
    {
    public:
        EthernetClient();
        EthernetClient(W5500& interface, const sock_id_e idx);
        ~EthernetClient();
    public:
        int connect(IPAddress ip, uint16_t port);
        int connect(IPAddress ip, uint16_t port, int32_t timeout_ms);
        int connect(const char* host, uint16_t port);
        int connect(const char* host, uint16_t port, int32_t timeout_ms);
    public:
        size_t write(uint8_t data);
        size_t write(const uint8_t *buf, size_t size);
        size_t write_P(PGM_P buf, size_t size);
        size_t write(Stream &stream);
    public:
        int available();
        int read();
        int read(uint8_t* buf, size_t size);
        int peek();
        void flush();
        void stop();
        uint8_t connected();
    public:
        operator bool()
        {
            return connected();
        }
        EthernetClient& operator=(const EthernetClient &other);
        bool operator==(const bool value)
        {
            return bool() == value;
        }
        bool operator!=(const bool value)
        {
            return bool() != value;
        }
        bool operator==(const EthernetClient&);
        bool operator!=(const EthernetClient& rhs)
        {
            return !this->operator==(rhs);
        };

    protected:
        std::shared_ptr<Socket> mSocket;
        std::shared_ptr<EthernetClientRxBuffer> mRxBuffer;
        bool mIsConnected;
        int mTimeout;
        Mutex mMutex;
    };

    extern EthernetClient* embededEthernetClient;
    extern EthernetClient* link1EthernetClient;
}}

#endif