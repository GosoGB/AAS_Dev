/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Ethernet.h"



namespace muffin { namespace jvs { namespace config {

    Ethernet::Ethernet()
        : Base(cfg_key_e::ETHERNET)
    {
        ASSERT(
            (
                mStaticIPv4 == INADDR_NONE &&
                mSubnetmask == INADDR_NONE &&
                mGateway    == INADDR_NONE &&
                mDNS1       == INADDR_NONE &&
                mDNS2       == INADDR_NONE
            ), "IP ADDRESSES MUST BE 0.0.0.0"
        );

        mSetFlags.reset();
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
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::IPv4))     == false &&
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::SUBNET))   == false &&
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::GATEWAY))  == false &&
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DNS_1))    == false &&
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DNS_2))    == false
            ), "STATIC IPv4 CANNOT BE SET PRIOR TO DHCP"
        );

        mEnableDHCP = enableDHCP;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DHCP));
    }

    void Ethernet::SetStaticIPv4(const IPAddress& staticIPv4)
    {
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
        ASSERT(
            (
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)) == true &&
                mEnableDHCP == false
            ), "DHCP MUST BE SET TO FALSE"
        );

        mStaticIPv4 = staticIPv4;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::IPv4));
    }

    void Ethernet::SetSubnetmask(const IPAddress& subnetmask)
    {
        ASSERT(
            (
                subnetmask != IPAddress(0, 0, 0, 0)        ||
                subnetmask != IPAddress(127, 0, 0, 1)      ||
                subnetmask != IPAddress(192, 0, 2, 0)      ||
                subnetmask != IPAddress(203, 0, 113, 0)    ||
                subnetmask != IPAddress(255, 255, 255, 255)
            ), "INVALID IPv4 ADDRESS"
        );
        ASSERT(
            (
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)) == true &&
                mEnableDHCP == false
            ), "DHCP MUST BE SET TO FALSE"
        );

        mSubnetmask = subnetmask;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::SUBNET));
    }

    void Ethernet::SetGateway(const IPAddress& gateway)
    {
        ASSERT(
            (
                gateway != IPAddress(0, 0, 0, 0)        ||
                gateway != IPAddress(127, 0, 0, 1)      ||
                gateway != IPAddress(192, 0, 2, 0)      ||
                gateway != IPAddress(203, 0, 113, 0)    ||
                gateway != IPAddress(255, 255, 255, 255)
            ), "INVALID IPv4 ADDRESS"
        );
        ASSERT(
            (
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)) == true &&
                mEnableDHCP == false
            ), "DHCP MUST BE SET TO FALSE"
        );

        mGateway = gateway;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::GATEWAY));
    }

    void Ethernet::SetDNS1(const IPAddress& dns1)
    {
        ASSERT(
            (
                dns1 != IPAddress(0, 0, 0, 0)        ||
                dns1 != IPAddress(127, 0, 0, 1)      ||
                dns1 != IPAddress(192, 0, 2, 0)      ||
                dns1 != IPAddress(203, 0, 113, 0)    ||
                dns1 != IPAddress(255, 255, 255, 255)
            ), "INVALID IPv4 ADDRESS"
        );
        ASSERT(
            (
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)) == true &&
                mEnableDHCP == false
            ), "DHCP MUST BE SET TO FALSE"
        );

        mDNS1 = dns1;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DNS_1));
    }

    void Ethernet::SetDNS2(const IPAddress& dns2)
    {
        ASSERT(
            (
                dns2 != IPAddress(0, 0, 0, 0)        ||
                dns2 != IPAddress(127, 0, 0, 1)      ||
                dns2 != IPAddress(192, 0, 2, 0)      ||
                dns2 != IPAddress(203, 0, 113, 0)    ||
                dns2 != IPAddress(255, 255, 255, 255)
            ), "INVALID IPv4 ADDRESS"
        );
        ASSERT(
            (
                mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)) == true &&
                mEnableDHCP == false
            ), "DHCP MUST BE SET TO FALSE"
        );

        mDNS2 = dns2;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DNS_2));
    }

    std::pair<Status, bool> Ethernet::GetDHCP() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DHCP)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::IPv4)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::SUBNET)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::GATEWAY)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DNS_1)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DNS_2)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mDNS2);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDNS2);
        }
    }
}}}