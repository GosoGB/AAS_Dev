/**
 * @file Operation.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-10-08
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Operation.h"



namespace muffin { namespace jarvis { namespace config {

    Operation::Operation()
        : Base(cfg_key_e::OPERATION)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Operation::~Operation()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

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
        mIsPlanExpiredSet = true;
    }

    void Operation::SetFactoryReset(const bool factoryReset)
    {
        mFactoryReset = factoryReset;
        mIsFactoryResetSet = true;
    }

    void Operation::SetServerNIC(const snic_e snic)
    {
        mServerNIC = snic;
        mIsServerNicSet = true;
    }

    void Operation::SetIntervalServer(const uint16_t interval)
    {
        ASSERT((interval > 0), "INTERVAL CANNOT BE SET TO 0");

        mIntervalServer = interval;
        mIsIntervalServerSet = true;
    }

    void Operation::SetIntervalPolling(const uint16_t interval)
    {
        ASSERT((interval > 0), "INTERVAL CANNOT BE SET TO 0");

        mIntervalPolling = interval;
        mIsIntervalPollingSet = true;
    }

    std::pair<Status, bool> Operation::GetPlanExpired() const
    {
        if (mIsPlanExpiredSet)
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
        if (mIsFactoryResetSet)
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
        if (mIsServerNicSet)
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
        if (mIsIntervalServerSet)
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
        if (mIsIntervalPollingSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mIntervalPolling);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIntervalPolling);
        }
    }
}}}