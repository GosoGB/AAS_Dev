/**
 * @file EthernetIP.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <IPAddress.h>
#include <string>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs { namespace config {

    class EthernetIP : public Base
    {
    public:
        EthernetIP();
        virtual ~EthernetIP() override;
    public:
        EthernetIP& operator=(const EthernetIP& obj);
        bool operator==(const EthernetIP& obj) const;
        bool operator!=(const EthernetIP& obj) const;
    public:
        void SetIPv4(const IPAddress& ipv4);
        void SetPort(const uint16_t prt);
        void SetNodes(std::vector<std::string>&& nodes) noexcept;
        void SetEthernetInterface(const if_e eth);

    public:
        std::pair<Status, if_e> GetEthernetInterface() const;
        std::pair<Status, IPAddress> GetIPv4() const;
        std::pair<Status, uint16_t> GetPort() const;
        std::pair<Status, std::vector<std::string>> GetNodes() const;
    private:
        bool mIsIPv4Set             = false;
        bool mIsNodesSet            = false;
        bool mIsPortSet             = false;
        bool mIsEthernetInterface   = false;
    private:
        if_e mEthernetInterface;
        IPAddress mIPv4;
        std::vector<std::string> mNodes;
        uint16_t mPort;
    };
}}}