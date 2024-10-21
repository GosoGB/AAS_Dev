/**
 * @file BrokerInfo.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 브로커의 연결 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <string.h>

#include "BrokerInfo.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace mqtt {
    
    BrokerInfo::BrokerInfo(const char* clientID, const char* host, const uint16_t port, const uint16_t keepalive, const socket_e socketID, const char* username, const char* password, const version_e version, const bool enableSSL)
        : mHost(host)
        , mPort(port)
        , mKeepAlive(keepalive)
        , mSocketID(socketID)
        , mUsername(username)
        , mPassword(password)
        , mVersion(version)
        , mClientID(clientID)
        , mEnableSSL(enableSSL)
    {
        ASSERT((strlen(host) < 101), "HOST NAME CAN'T EXCEED 100 BYTES");
        ASSERT((0 < port), "INVALID PORT NUMBER");
        ASSERT((keepalive < 3601), "INVALID KEEP ALIVE");
        ASSERT((0 < strlen(clientID) && strlen(clientID) < 24), "CLIENT ID IS INVALID");

    #if defined(DEBUG)
        char buffer[1024] = {0};
        sprintf(buffer, "\n \
--------------------------------------------------\n \
Constructed at address: %p\n \
--------------------------------------------------\n \
Host: %s\n \
Port: %u\n \
KeepAlive: %u\n \
User: %s\n \
Pass: %s\n \
Version: %s\n \
Client ID: %s\n \
SSL Enabled: %s\n \
Socket ID: %u\n \
--------------------------------------------------\n\n"
, this, GetHost(), GetPort(), GetKeepAlive(), GetUsername(), GetPassword(), 
GetVersion() == version_e::Ver_3_1_0 ? "3.1.0" : "3.1.1",
GetClientID(), IsSslEnabled() ? "true" : "false", static_cast<uint8_t>(GetSocketID()));
        LOG_VERBOSE(logger, "%s", buffer);
    #endif
    }

    BrokerInfo::BrokerInfo(const BrokerInfo&& obj) noexcept
        : mHost(std::move(obj.mHost))
        , mPort(std::move(obj.mPort))
        , mKeepAlive(std::move(obj.mKeepAlive))
        , mSocketID(std::move(obj.mSocketID))
        , mUsername(std::move(obj.mUsername))
        , mPassword(std::move(obj.mPassword))
        , mVersion(std::move(obj.mVersion))
        , mClientID(std::move(obj.mClientID))
        , mEnableSSL(std::move(obj.mEnableSSL))
    {
    #if defined(DEBUG)
        char buffer[1024] = {0};
        sprintf(buffer, "\n \
--------------------------------------------------\n \
Constructed by Move from %p to %p\n \
--------------------------------------------------\n \
Host: %s\n \
Port: %u\n \
KeepAlive: %u\n \
User: %s\n \
Pass: %s\n \
Version: %s\n \
Client ID: %s\n \
SSL Enabled: %s\n \
Socket ID: %u\n \
--------------------------------------------------\n\n"
, &obj, this, GetHost(), GetPort(), GetKeepAlive(), GetUsername(), GetPassword(), 
GetVersion() == version_e::Ver_3_1_0 ? "3.1.0" : "3.1.1",
GetClientID(), IsSslEnabled() ? "true" : "false", static_cast<uint8_t>(GetSocketID()));
        LOG_VERBOSE(logger, "%s", buffer);
    #endif
    }

    BrokerInfo::~BrokerInfo()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    BrokerInfo& BrokerInfo::operator=(const BrokerInfo& obj)
    {
        if (this != &obj)
        {
            mHost        = obj.mHost;
            mPort        = obj.mPort;
            mKeepAlive   = obj.mKeepAlive;
            mSocketID    = obj.mSocketID;
            mUsername    = obj.mUsername;
            mPassword    = obj.mPassword;
            mVersion     = obj.mVersion;
            mClientID    = obj.mClientID;
            mEnableSSL   = obj.mEnableSSL;
        }

        return *this;
    }

    bool BrokerInfo::operator==(const BrokerInfo& obj)
    {
        return (
            mHost        == obj.mHost         &&
            mPort        == obj.mPort         &&
            mKeepAlive   == obj.mKeepAlive    &&
            mSocketID    == obj.mSocketID     &&
            mUsername    == obj.mUsername     &&
            mPassword    == obj.mPassword     &&
            mVersion     == obj.mVersion      &&
            mClientID    == obj.mClientID     &&
            mEnableSSL   == obj.mEnableSSL
        );
    }

    bool BrokerInfo::operator!=(const BrokerInfo& obj)
    {
        return !(*this == obj);
    }

    Status BrokerInfo::SetHost(const std::string& host)
    {
        ASSERT((host.length() < 101), "HOST NAME CAN'T EXCEED 100 BYTES");

        if (host.length() < 101)
        {
            mHost = host;
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "HOST IS TOO LONG");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status BrokerInfo::SetPort(const uint16_t port)
    {
        ASSERT((0 < port), "PORT NUMBER MUST BE GREATER THAN 0");

        if (0 < port)
        {
            mPort = port;
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "PORT CANNOT BE 0");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status BrokerInfo::SetKeepAlive(const uint16_t keepalive)
    {
        ASSERT((keepalive < 3601), "KEEP ALIVE CANNOT EXCEED 3,600");

        if (keepalive < 3601)
        {
            mKeepAlive = keepalive;
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "KEEP ALIVE CANNOT EXCEED 3,600");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status BrokerInfo::SetSocketID(const socket_e socketID)
    {
        mSocketID = socketID;
        return Status(Status::Code::GOOD);
    }

    Status BrokerInfo::SetUsername(const std::string& username)
    {
        mUsername = username;
        return Status(Status::Code::GOOD);
    }

    Status BrokerInfo::SetPassword(const std::string& password)
    {
        mPassword = password;
        return Status(Status::Code::GOOD);
    }

    Status BrokerInfo::SetVersion(const version_e version)
    {
        mVersion = version;
        return Status(Status::Code::GOOD);
    }

    Status BrokerInfo::SetClientID(const std::string& clientID)
    {
        ASSERT((0 < clientID.length() && clientID.length() < 24), "CLIENT ID LENGTH CANNOT BE 0 OR LONGER THAN 23");

        if (0 < clientID.length() && clientID.length() < 24)
        {
            mClientID = clientID;
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "CLIENT ID LENGTH CANNOT BE 0 OR LONGER THAN 23");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status BrokerInfo::EnableSSL(const bool enableSSL)
    {
        mEnableSSL = enableSSL;
        return Status(Status::Code::GOOD);
    }

    const char* BrokerInfo::GetHost() const
    {
        return mHost.c_str();
    }

    uint16_t BrokerInfo::GetPort() const
    {
        return mPort;
    }

    uint16_t BrokerInfo::GetKeepAlive() const
    {
        return mKeepAlive;
    }

    const char* BrokerInfo::GetUsername() const
    {
        if (mUsername.length() == 0)
        {
            return nullptr;
        }
        else
        {
            return mUsername.c_str();
        }
    }

    const char* BrokerInfo::GetPassword() const
    {
        if (mPassword.length() == 0)
        {
            return nullptr;
        }
        else
        {
            return mPassword.c_str();
        }
    }

    version_e BrokerInfo::GetVersion() const
    {
        return mVersion;
    }
    
    const char* BrokerInfo::GetClientID() const
    {
        return mClientID.c_str();
    }

    bool BrokerInfo::IsSslEnabled() const
    {
        return mEnableSSL;
    }

    socket_e BrokerInfo::GetSocketID() const
    {
        return mSocketID;
    }
}}