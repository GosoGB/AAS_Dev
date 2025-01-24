/**
 * @file Operation.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Operation.h"



namespace muffin { namespace jvs { namespace config {

    Operation& Operation::operator=(const Operation& obj)
    {
        if (this != &obj)
        {
            mPlanExpired        = obj.mPlanExpired;
            mFactoryReset       = obj.mFactoryReset;
            mServerNIC          = obj.mServerNIC;
            mIntervalServer     = obj.mIntervalServer;
            mIntervalPolling    = obj.mIntervalPolling;
        }
        return *this;
    }

    bool Operation::operator==(const Operation& obj) const
    {
        return (
            mPlanExpired     == obj.mPlanExpired      &&
            mFactoryReset    == obj.mFactoryReset     &&
            mServerNIC       == obj.mServerNIC        &&
            mIntervalServer  == obj.mIntervalServer   &&
            mIntervalPolling == obj.mIntervalPolling
        );
    }

    bool Operation::operator!=(const Operation& obj) const
    {
        return !(*this == obj);
    }

    void Operation::SetPlanExpired(const bool planExpired)
    {
        mPlanExpired = planExpired;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::SERVICE_PLAN));
    }

    void Operation::SetFactoryReset(const bool factoryReset)
    {
        mFactoryReset = factoryReset;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::FACTORY_RESET));
    }

    void Operation::SetServerNIC(const snic_e snic)
    {
        mServerNIC = snic;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::SERVICE_NIC));
    }

    void Operation::SetIntervalServer(const uint16_t interval)
    {
        ASSERT((interval > 0), "INTERVAL CANNOT BE SET TO 0");

        mIntervalServer = interval;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::PUB_INTERVAL));
    }

    void Operation::SetIntervalPolling(const uint16_t interval)
    {
        ASSERT((interval > 0), "INTERVAL CANNOT BE SET TO 0");

        mIntervalPolling = interval;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DAQ_INTERVAL));
    }

    std::pair<Status, bool> Operation::GetPlanExpired() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::SERVICE_PLAN)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mPlanExpired);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPlanExpired);
        }
    }

    std::pair<Status, bool> Operation::GetFactoryReset() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::FACTORY_RESET)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mFactoryReset);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mFactoryReset);
        }
    }

    std::pair<Status, snic_e> Operation::GetServerNIC() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::SERVICE_NIC)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mServerNIC);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mServerNIC);
        }
    }

    std::pair<Status, uint16_t> Operation::GetIntervalServer() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::PUB_INTERVAL)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mIntervalServer);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIntervalServer);
        }
    }

    std::pair<Status, uint16_t> Operation::GetIntervalPolling() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DAQ_INTERVAL)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mIntervalPolling);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIntervalPolling);
        }
    }


    Operation operation;
}}}