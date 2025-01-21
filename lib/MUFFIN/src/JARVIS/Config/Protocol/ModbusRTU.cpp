/**
 * @file ModbusRTU.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusRTU.h"



namespace muffin { namespace jvs { namespace config {

    ModbusRTU::ModbusRTU()
        : Base(cfg_key_e::MODBUS_RTU)
    {
    }

    ModbusRTU::~ModbusRTU()
    {
    }

    ModbusRTU& ModbusRTU::operator=(const ModbusRTU& obj)
    {
        if (this != &obj)
        {
            mNodes    = obj.mNodes;
            mPort     = obj.mPort;
            mSlaveID  = obj.mSlaveID;
        }
        
        return *this;
    }

    bool ModbusRTU::operator==(const ModbusRTU& obj) const
    {
       return (
            mNodes    == obj.mNodes   && 
            mPort     == obj.mPort    &&
            mSlaveID  == obj.mSlaveID
        );
    }

    bool ModbusRTU::operator!=(const ModbusRTU& obj) const
    {
        return !(*this == obj);
    }

    void ModbusRTU::SetPort(const prt_e prt)
    {
        mPort = prt;
        mIsPortSet = true;
    }

    void ModbusRTU::SetSlaveID(const uint8_t sid)
    {
        ASSERT((0 < sid && sid < 248), "INVALID SLAVED ID: %u", sid);
        mSlaveID = sid;
        mIsSlaveIdSet = true;
    }

    void ModbusRTU::SetNodes(std::vector<std::string>&& nodes) noexcept
    {
        ASSERT((nodes.size() != 0), "NODE REFERENCES CANNOT BE NULL");

        mNodes = std::move(nodes);
        mIsNodesSet = true;
    }

    std::pair<Status, prt_e> ModbusRTU::GetPort() const
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

    std::pair<Status, uint8_t> ModbusRTU::GetSlaveID() const
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

    std::pair<Status, std::vector<std::string>> ModbusRTU::GetNodes() const
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