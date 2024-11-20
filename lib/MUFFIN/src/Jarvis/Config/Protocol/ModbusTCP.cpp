/**
 * @file ModbusTCP.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusTCP.h"



namespace muffin { namespace jarvis { namespace config {

    ModbusTCP::ModbusTCP()
        : Base(cfg_key_e::MODBUS_TCP)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    ModbusTCP::~ModbusTCP()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    ModbusTCP& ModbusTCP::operator=(const ModbusTCP& obj)
    {
        if (this != &obj)
        {
            mNIC        = obj.mNIC;
            mIPv4       = obj.mIPv4;
            mNodes      = obj.mNodes;
            mPort       = obj.mPort;
            mSlaveID    = obj.mSlaveID;
        }

        return *this;
    }

    bool ModbusTCP::operator==(const ModbusTCP& obj) const
    {
       return (
            mNIC        == obj.mNIC      &&
            mIPv4       == obj.mIPv4     &&
            mNodes      == obj.mNodes    &&
            mPort       == obj.mPort     &&
            mSlaveID    == obj.mSlaveID
        );
    }

    bool ModbusTCP::operator!=(const ModbusTCP& obj) const
    {
        return !(*this == obj);
    }

    void ModbusTCP::SetNIC(const nic_e iface)
    {
        ASSERT((iface != nic_e::LTE_CatM1), "LTE CANNOT BE CHOSEN AS AN INTERFACE");

        mNIC = iface;
        mIsNicSet = true;
    }

    void ModbusTCP::SetIPv4(const IPAddress& ipv4)
    {
        ASSERT(
            (
                ipv4 != IPAddress(0, 0, 0, 0)        ||
                ipv4 != IPAddress(127, 0, 0, 1)      ||
                ipv4 != IPAddress(192, 0, 2, 0)      ||
                ipv4 != IPAddress(203, 0, 113, 0)    ||
                ipv4 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mIPv4 = ipv4;
        mIsIPv4Set = true;
    }

    void ModbusTCP::SetPort(const uint16_t prt)
    {
        ASSERT((0 != prt), "INVALID PORT NUMBER");

        mPort = prt;
        mIsPortSet = true;
    }

    void ModbusTCP::SetSlaveID(const uint8_t sid)
    {
        ASSERT((0 < sid && sid < 248), "INVALID SLAVED ID: %u", sid);

        mSlaveID = sid;
        mIsSlaveIdSet = true;
    }

    void ModbusTCP::SetNodes(std::vector<std::string>&& nodes) noexcept
    {
        ASSERT((nodes.size() != 0), "NODE REFERENCES CANNOT BE NULL");

        mNodes = std::move(nodes);
        mIsNodesSet = true;
    }

    std::pair<Status, nic_e> ModbusTCP::GetNIC() const
    {
        if (mIsNicSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNIC);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNIC);
        }
    }

    std::pair<Status, IPAddress> ModbusTCP::GetIPv4() const
    {
        if (mIsIPv4Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mIPv4);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIPv4);
        }
    }

    std::pair<Status, uint16_t> ModbusTCP::GetPort() const
    {
        if (mIsPortSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mPort);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPort);
        }
    }

    std::pair<Status, uint8_t> ModbusTCP::GetSlaveID() const
    {
        if (mIsSlaveIdSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mSlaveID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mSlaveID);
        }
    }

    std::pair<Status, std::vector<std::string>> ModbusTCP::GetNodes() const
    {
        if (mIsNodesSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodes);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodes);
        }
    }
}}}