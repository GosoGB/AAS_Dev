/**
 * @file Ethernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <IPAddress.h>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class Ethernet : public Base
    {
    public:
        Ethernet();
        virtual ~Ethernet() override;
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
        bool mIsEnableDhcpSet = false;
        bool mIsStaticIPv4Set = false;
        bool mIsSubnetmaskSet = false;
        bool mIsGatewaySet    = false;
        bool mIsDNS1Set       = false;
        bool mIsDNS2Set       = false;
    private:
        bool mEnableDHCP;
        IPAddress mStaticIPv4;
        IPAddress mSubnetmask;
        IPAddress mGateway;
        IPAddress mDNS1;
        IPAddress mDNS2;
    };
}}}