/**
 * @file ModbusTCP.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <IPAddress.h>
#include <string>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class ModbusTCP : public Base
    {
    public:
        ModbusTCP();
        virtual ~ModbusTCP() override;
    public:
        ModbusTCP& operator=(const ModbusTCP& obj);
        bool operator==(const ModbusTCP& obj) const;
        bool operator!=(const ModbusTCP& obj) const;
    public:
        void SetNIC(const nic_e iface);
        void SetIPv4(const IPAddress& ipv4);
        void SetPort(const uint16_t prt);
        void SetSlaveID(const uint8_t sid);
        void SetNodes(std::vector<std::string>&& nodes) noexcept;
    public:
        std::pair<Status, nic_e> GetNIC() const;
        std::pair<Status, IPAddress> GetIPv4() const;
        std::pair<Status, uint16_t> GetPort() const;
        std::pair<Status, uint8_t> GetSlaveID() const;
        std::pair<Status, std::vector<std::string>> GetNodes() const;
    private:
        bool mIsNicSet = false;
        bool mIsIPv4Set      = false;
        bool mIsNodesSet     = false;
        bool mIsPortSet    = false;
        bool mIsSlaveIdSet   = false;
    private:
        nic_e mNIC;
        IPAddress mIPv4;
        std::vector<std::string> mNodes;
        uint16_t mPort;
        uint8_t mSlaveID;
    };
}}}