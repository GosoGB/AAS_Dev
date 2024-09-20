/**
 * @file ModbusRTU.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-04
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusRTU.h"



namespace muffin { namespace jarvis { namespace config {

    ModbusRTU::ModbusRTU(const std::string& key)
        : Base(key)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    ModbusRTU::~ModbusRTU()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
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

    void ModbusRTU::SetNodes(std::vector<std::string>&& nodes) noexcept
    {
        ASSERT((nodes.size() != 0), "NODE REFERENCES CANNOT BE NULL");

        mNodes = std::move(nodes);
        mIsNodesSet = true;
    }

    void ModbusRTU::SetPort(const prt_e prt)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((static_cast<uint8_t>(prt) == 2), "UNDEFINED SERIAL PORT");
    #else
        ASSERT(
            (
                static_cast<uint8_t>(prt) == 2 || 
                static_cast<uint8_t>(prt) == 3
            ), 
            "UNDEFINED SERIAL PORT"
        );
    #endif
    
        mPort = prt;
        mIsPortSet = true;
    }

    void ModbusRTU::SetSlaveID(const uint8_t sid)
    {
        ASSERT((0 < sid && sid < 248), "INVALID SLAVED ID: %u", sid);

        mSlaveID = sid;
        mIsSlaveIdSet = true;
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

    std::pair<Status, uint8_t> ModbusRTU::GetPort() const
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
            return std::make_pair(Status(Status::Code::GOOD), mIsSlaveIdSet);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIsSlaveIdSet);
        }
    }
}}}