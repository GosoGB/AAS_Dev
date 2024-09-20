/**
 * @file Production.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Logger/Logger.h"
#include "Production.h"


namespace muffin { namespace jarvis { namespace config {

    Production::Production()
        : Base("prod")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Production::~Production()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Production& Production::operator=(const Production& obj)
    {
        if (this != &obj)
        {
            mTotalProductionNodeID     =  obj.mTotalProductionNodeID;       
            mGoodQualityNodeID         =  obj.mGoodQualityNodeID;
            mDefectNodeID              =  obj.mDefectNodeID;
        }
        
        return *this;
    }

    bool Production::operator==(const Production& obj) const
    {
       return (
            mTotalProductionNodeID     ==  obj.mTotalProductionNodeID     &&       
            mGoodQualityNodeID         ==  obj.mGoodQualityNodeID         &&
            mDefectNodeID              ==  obj.mDefectNodeID   
        );
    }

    bool Production::operator!=(const Production& obj) const
    {
        return !(*this == obj);
    }

    Status Production::SetTotalProductionNodeID(const std::string& nodeID)
    {
        mTotalProductionNodeID = nodeID;
        if (mTotalProductionNodeID == nodeID)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Production::SetGoodQualityNodeID(const std::string& nodeID)
    {
        mGoodQualityNodeID = nodeID;
        if (mGoodQualityNodeID == nodeID)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Production::SetDefectNodeID(const std::string& nodeID)
    {
        mDefectNodeID = nodeID;
        if (mDefectNodeID == nodeID)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    const std::string& Production::GetTotalProductionNodeID() const
    {
        return mTotalProductionNodeID;
    }

    const std::string& Production::GetGoodQualityNodeID() const
    {
        return mGoodQualityNodeID;
    }

    const std::string& Production::GetDefectNodeID() const
    {
        return mDefectNodeID;
    }

  
}}}


