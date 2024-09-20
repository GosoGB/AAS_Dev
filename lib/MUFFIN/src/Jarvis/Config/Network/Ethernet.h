/**
 * @file Ethernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <IPAddress.h>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



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
        Status SetDHCP(const bool enableDHCP);
        Status SetStaticIPv4(const std::string& staticIPv4);
        Status SetSubnet(const std::string& subnetmask);
        Status SetGateway(const std::string& gateway);
        Status SetDNS1(const std::string& dns1);
        Status SetDNS2(const std::string& dns2);
    public:
        bool GetDHCP() const;
        const IPAddress& GetStaticIPv4() const;
        const IPAddress& GetSubnet() const;
        const IPAddress& GetGateway() const;
        const IPAddress& GetDNS1() const;
        const IPAddress& GetDNS2() const;
    private:
        bool mEnableDHCP;
        IPAddress mStaticIPv4;
        IPAddress mSubnetmask;
        IPAddress mGateway;
        IPAddress mDNS1;
        IPAddress mDNS2;
    };
}}}