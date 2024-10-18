/**
 * @file BrokerInfo.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 브로커의 연결 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    class BrokerInfo
    {
    public:
        BrokerInfo(const char* clientID, const char* host = "broker.edgecross.ai", const uint16_t port = 8883, const uint16_t keepalive = 40, const socket_e socketID = socket_e::SOCKET_0, const char* username = "vitcon", const char* password = "!1vola2proutr09@", const version_e version = version_e::Ver_3_1_1, const bool enableSSL = true);
        BrokerInfo(const BrokerInfo&& obj) noexcept;
        virtual ~BrokerInfo();
    public:
        BrokerInfo& operator=(const BrokerInfo& obj);
        bool operator==(const BrokerInfo& obj);
        bool operator!=(const BrokerInfo& obj);
    public:
        Status SetHost(const std::string& host);
        Status SetPort(const uint16_t port);
        Status SetKeepAlive(const uint16_t keepalive);
        Status SetSocketID(const socket_e socketID);
        Status SetUsername(const std::string& username);
        Status SetPassword(const std::string& password);
        Status SetVersion(const version_e version);
        Status SetClientID(const std::string& clientID);
        Status EnableSSL(const bool enableSSL);
    public:
        const char* GetHost() const;
        uint16_t GetPort() const;
        uint16_t GetKeepAlive() const;
        socket_e GetSocketID() const;
        const char* GetUsername() const;
        const char* GetPassword() const;
        version_e GetVersion() const;
        const char* GetClientID() const;
        bool IsSslEnabled() const;
    private:
        std::string mHost;
        uint16_t mPort;
        uint16_t mKeepAlive;
        socket_e mSocketID;
        std::string mUsername;
        std::string mPassword;
        version_e mVersion;
        std::string mClientID;
        bool mEnableSSL;
    };
}}