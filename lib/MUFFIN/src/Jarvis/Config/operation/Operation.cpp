/**
 * @file Operation.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Operation.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace jarvis { namespace config {

    Operation::Operation()
        : Base("op")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Operation::~Operation()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Operation& Operation::operator=(const Operation& obj)
    {
        if (this != &obj)
        {
            mExpired         = obj.mExpired;
            mOTA             = obj.mOTA;
            mServerNIC       = obj.mServerNIC;
            mIntervalServer  = obj.mIntervalServer;
            mIntervalPolling = obj.mIntervalPolling;
        }
        
        return *this;
    }

    bool Operation::operator==(const Operation& obj) const
    {
        return (
            mExpired         == obj.mExpired          &&  
            mOTA             == obj.mOTA              &&
            mServerNIC       == obj.mServerNIC        &&  
            mIntervalServer  == obj.mIntervalServer   &&      
            mIntervalPolling == obj.mIntervalPolling            
        );
    }

    bool Operation::operator!=(const Operation& obj) const
    {
        return !(*this == obj);
    }

    Status Operation::SetExpired(const bool exp)
    {
        mExpired = exp;
        if (mExpired == exp)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Operation::SetOTA(const bool ota)
    {
        mOTA = ota;
        if (mOTA == ota)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Operation::SetIntervalServer(const uint32_t& intvsrv)
    {
        mIntervalServer = intvsrv;
        if (mIntervalServer == intvsrv)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Operation::SetServerNIC(const std::string& snic)
    {
        mServerNIC = snic;
        if (mServerNIC == snic)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    bool Operation::GetExpired() const
    {
        return mExpired;
    }

    bool Operation::GetOTA() const
    {
        return mOTA;
    }

    const uint32_t& Operation::GetIntervalServer() const
    {
        return mIntervalServer;
    }

    const uint16_t& Operation::GetIntervalPolling() const
    {
        return mIntervalPolling;
    }

    const std::string& Operation::GetServerNIC() const
    {
        return mServerNIC;
    }

   
}}}