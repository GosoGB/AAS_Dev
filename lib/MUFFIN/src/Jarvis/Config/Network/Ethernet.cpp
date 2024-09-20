/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Ethernet.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace jarvis { namespace config {

    Ethernet::Ethernet()
        : Base("ethernet")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Ethernet::~Ethernet()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Ethernet& Ethernet::operator=(const Ethernet& obj)
    {
        if (this != &obj)
        {
            mEnableDHCP = obj.mEnableDHCP;
            mStaticIPv4 = obj.mStaticIPv4;
            mSubnetmask = obj.mSubnetmask;
            mGateway    = obj.mGateway;
            mDNS1       = obj.mDNS1;
            mDNS2       = obj.mDNS2;
        }

        return *this;
    }

    bool Ethernet::operator==(const Ethernet& obj) const
    {
        return (
            mEnableDHCP == obj.mEnableDHCP &&
            mStaticIPv4 == obj.mStaticIPv4 &&
            mSubnetmask == obj.mSubnetmask &&
            mGateway    == obj.mGateway    &&
            mDNS1       == obj.mDNS1       &&
            mDNS2       == obj.mDNS2
        );
    }

    bool Ethernet::operator!=(const Ethernet& obj) const
    {
        return !(*this == obj);
    }

    Status Ethernet::SetDHCP(const bool enableDHCP)
    {
        mEnableDHCP = enableDHCP;
        if (mEnableDHCP == enableDHCP)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Ethernet::SetStaticIPv4(const std::string& staticIPv4)
    {
        if (mStaticIPv4.fromString(staticIPv4.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status Ethernet::SetSubnet(const std::string& subnetmask)
    {
        if (mSubnetmask.fromString(subnetmask.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status Ethernet::SetGateway(const std::string& gateway)
    {
        if (mGateway.fromString(gateway.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status Ethernet::SetDNS1(const std::string& dns1)
    {
        if (mDNS1.fromString(dns1.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status Ethernet::SetDNS2(const std::string& dns2)
    {
        if (mDNS2.fromString(dns2.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    bool Ethernet::GetDHCP() const
    {
        return mEnableDHCP;
    }

    const IPAddress& Ethernet::GetStaticIPv4() const
    {
        return mStaticIPv4;
    }

    const IPAddress& Ethernet::GetSubnet() const
    {
        return mSubnetmask;
    }

    const IPAddress& Ethernet::GetGateway() const
    {
        return mGateway;
    }

    const IPAddress& Ethernet::GetDNS1() const
    {
        return mDNS1;
    }

    const IPAddress& Ethernet::GetDNS2() const
    {
        return mDNS2;
    }
}}}