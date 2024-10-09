/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Ethernet.h"



namespace muffin { namespace jarvis { namespace config {

    Ethernet::Ethernet(const cfg_key_e category)
        : Base(category)
    {
    #if defined(DEBUG)
        ASSERT((category != cfg_key_e::ETHERNET), "CATEGORY DOES NOT MATCH");
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }

    Ethernet::~Ethernet()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
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

    void Ethernet::SetDHCP(const bool enableDHCP)
    {
        ASSERT(
            (
                mIsStaticIPv4Set == false &&
                mIsSubnetmaskSet == false &&
                mIsGatewaySet    == false &&
                mIsDNS1Set       == false &&
                mIsDNS2Set       == false
            ), "INVALID PRECONDITION: CANNOT SET STATIC IPv4 PRIOR TO DHCP"
        );

        mEnableDHCP = enableDHCP;
        mIsEnableDhcpSet = true;

        ASSERT(
            (
                mStaticIPv4 == INADDR_NONE &&
                mSubnetmask == INADDR_NONE &&
                mGateway    == INADDR_NONE &&
                mDNS1       == INADDR_NONE &&
                mDNS2       == INADDR_NONE
            ), "INVALID POSTCONDITION: IPv4 ADDRESSES MUST BE DEFAULT VALUE WHICH IS 0.0.0.0"
        );
    }

    void Ethernet::SetStaticIPv4(const IPAddress& staticIPv4)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                staticIPv4 != IPAddress(0, 0, 0, 0)        ||
                staticIPv4 != IPAddress(127, 0, 0, 1)      ||
                staticIPv4 != IPAddress(192, 0, 2, 0)      ||
                staticIPv4 != IPAddress(203, 0, 113, 0)    ||
                staticIPv4 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mStaticIPv4 = staticIPv4;
        mIsStaticIPv4Set = true;
    }

    void Ethernet::SetSubnetmask(const IPAddress& subnetmask)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                subnetmask != IPAddress(0, 0, 0, 0)        ||
                subnetmask != IPAddress(127, 0, 0, 1)      ||
                subnetmask != IPAddress(192, 0, 2, 0)      ||
                subnetmask != IPAddress(203, 0, 113, 0)    ||
                subnetmask != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mSubnetmask = subnetmask;
        mIsSubnetmaskSet = true;
    }

    void Ethernet::SetGateway(const IPAddress& gateway)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                gateway != IPAddress(0, 0, 0, 0)        ||
                gateway != IPAddress(127, 0, 0, 1)      ||
                gateway != IPAddress(192, 0, 2, 0)      ||
                gateway != IPAddress(203, 0, 113, 0)    ||
                gateway != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mGateway = gateway;
        mIsGatewaySet = true;
    }

    void Ethernet::SetDNS1(const IPAddress& dns1)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                dns1 != IPAddress(0, 0, 0, 0)        ||
                dns1 != IPAddress(127, 0, 0, 1)      ||
                dns1 != IPAddress(192, 0, 2, 0)      ||
                dns1 != IPAddress(203, 0, 113, 0)    ||
                dns1 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mDNS1 = dns1;
        mIsDNS1Set = true;
    }

    void Ethernet::SetDNS2(const IPAddress& dns2)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                dns2 != IPAddress(0, 0, 0, 0)        ||
                dns2 != IPAddress(127, 0, 0, 1)      ||
                dns2 != IPAddress(192, 0, 2, 0)      ||
                dns2 != IPAddress(203, 0, 113, 0)    ||
                dns2 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mDNS2 = dns2;
        mIsDNS2Set = true;
    }

    std::pair<Status, bool> Ethernet::GetDHCP() const
    {
        if (mIsEnableDhcpSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEnableDHCP);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEnableDHCP);
        }
    }

    std::pair<Status, IPAddress> Ethernet::GetStaticIPv4() const
    {
        if (mIsStaticIPv4Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mStaticIPv4);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mStaticIPv4);
        }
    }

    std::pair<Status, IPAddress> Ethernet::GetSubnetmask() const
    {
        if (mIsSubnetmaskSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mSubnetmask);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mSubnetmask);
        }
    }

    std::pair<Status, IPAddress> Ethernet::GetGateway() const
    {
        if (mIsGatewaySet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mGateway);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mGateway);
        }
    }

    std::pair<Status, IPAddress> Ethernet::GetDNS1() const
    {
        if (mIsDNS1Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDNS1);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDNS1);
        }
    }

    std::pair<Status, IPAddress> Ethernet::GetDNS2() const
    {
        if (mIsDNS2Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDNS2);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDNS2);
        }
    }
}}}