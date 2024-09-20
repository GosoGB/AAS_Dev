/**
 * @file Optime.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Logger/Logger.h"
#include "Optime.h"


namespace muffin { namespace jarvis { namespace config {

    Optime::Optime()
        : Base("optime")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Optime::~Optime()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Optime& Optime::operator=(const Optime& obj)
    {
        if (this != &obj)
        {
            mNodeID     =  obj.mNodeID;       
            mOptimeType =  obj.mOptimeType;
            mCriterion  =  obj.mCriterion;
            mOperater   =  obj.mOperater;
        }
        
        return *this;
    }

    bool Optime::operator==(const Optime& obj) const
    {
       return (
            mNodeID     ==  obj.mNodeID      &&       
            mOptimeType ==  obj.mOptimeType  &&
            mCriterion  ==  obj.mCriterion   &&
            mOperater   ==  obj.mOperater
        );
    }

    bool Optime::operator!=(const Optime& obj) const
    {
        return !(*this == obj);
    }

    Status Optime::SetNodeID(const std::string& nodeID)
    {
        mNodeID = nodeID;
        if (mNodeID == nodeID)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Optime::SetOptimeType(const uint8_t& type)
    {
        mOptimeType = type;
        if (mOptimeType == type)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Optime::SetCriterion(const uint32_t& criterion)
    {
        mCriterion = criterion;
        if (mCriterion == criterion)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Optime::SetOperator(const std::string& operater)
    {
        mOperater = operater;
        if (mOperater == operater)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    const std::string& Optime::GetNodeID() const
    {
        return mNodeID;
    }

    const uint8_t& Optime::GetOptimeType() const
    {
        return mOptimeType;
    }

    const uint32_t& Optime::GetCriterion() const
    {
        return mCriterion;
    }

    const std::string& Optime::GetOperator() const
    {
        return mOperater;
    }
}}}


