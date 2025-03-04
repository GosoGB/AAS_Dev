/**
 * @file Ethernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <IPAddress.h>

#include "Common/Status.h"
#include "Common/DataStructure/bitset.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs { namespace config {

    class Ethernet : public Base
    {
    public:
        Ethernet();
        virtual ~Ethernet() override {}
    public:
        Ethernet& operator=(const Ethernet& obj);
        bool operator==(const Ethernet& obj) const;
        bool operator!=(const Ethernet& obj) const;
    public:
        void SetDHCP(const bool enableDHCP);
        void SetStaticIPv4(const IPAddress& staticIPv4);
        void SetSubnetmask(const IPAddress& subnetmask);
        void SetGateway(const IPAddress& gateway);
        void SetDNS1(const IPAddress& dns1);
        void SetDNS2(const IPAddress& dns2);
    public:
        std::pair<Status, bool> GetDHCP() const;
        std::pair<Status, IPAddress> GetStaticIPv4() const;
        std::pair<Status, IPAddress> GetSubnetmask() const;
        std::pair<Status, IPAddress> GetGateway() const;
        std::pair<Status, IPAddress> GetDNS1() const;
        std::pair<Status, IPAddress> GetDNS2() const;
    private:
        typedef enum class SetFlagEnum
            : uint8_t
        {
            DHCP     = 0,
            IPv4     = 1,
            SUBNET   = 2,
            GATEWAY  = 3,
            DNS_1    = 4,
            DNS_2    = 5,
            TOP      = 6
        } set_flag_e;
        bitset<static_cast<uint8_t>(set_flag_e::TOP)> mSetFlags;
    private:
        bool mEnableDHCP;
        IPAddress mStaticIPv4;
        IPAddress mSubnetmask;
        IPAddress mGateway;
        IPAddress mDNS1;
        IPAddress mDNS2;
    };


    extern Ethernet* ethernet;
}}}