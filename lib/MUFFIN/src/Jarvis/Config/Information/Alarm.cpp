/**
 * @file Alarm.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Logger/Logger.h"
#include "Alarm.h"


namespace muffin { namespace jarvis { namespace config {

    Alarm::Alarm(const std::string& key)
        : Base(key)
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Alarm::~Alarm()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Alarm& Alarm::operator=(const Alarm& obj)
    {
        if (this != &obj)
        {
            mCondition  =  obj.mCondition;
            mNodeID     =  obj.mNodeID;
            mAlarmType  =  obj.mAlarmType;
            mUCL        =  obj.mUCL;
            mLCL        =  obj.mLCL;
            // mLclUID     =  obj.mLclUID;
            // mUclUID     =  obj.mUclUID;
            // mLclPID     =  obj.mLclPID;
            // mUclPID     =  obj.mUclPID;
        }
        
        return *this;
    }

    bool Alarm::operator==(const Alarm& obj) const
    {
       return (
            mCondition  ==  obj.mCondition  &&       
            mNodeID     ==  obj.mNodeID     &&
            mAlarmType  ==  obj.mAlarmType  &&
            mUCL        ==  obj.mUCL        &&
            mLCL        ==  obj.mLCL  
            // mLclUID     ==  obj.mLclUID     && 
            // mUclUID     ==  obj.mUclUID     && 
            // mLclPID     ==  obj.mLclPID     && 
            // mUclPID     ==  obj.mUclPID     && 
        );
    }

    bool Alarm::operator!=(const Alarm& obj) const
    {
        return !(*this == obj);
    }

    Status Alarm::SetNodeID(const std::string& nodeID)
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

    Status Alarm::SetAlarmType(const uint8_t& type)
    {
        mAlarmType = type;
        if (mAlarmType == type)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Alarm::SetUCL(const double& ucl)
    {
        mUCL = ucl;
        if (mUCL == ucl)
        {
            mIsUclSet = true;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Alarm::SetLCL(const double& lcl)
    {
        mLCL = lcl;
        if (mLCL == lcl)
        {
            mIsLclSet = true;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Alarm::SetCondition(const std::vector<uint16_t>& condition)
    {
        mCondition = condition;
        if (mCondition == condition)
        {
            mHasCondition = true;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    // Status Alarm::SetUclUID(const std::string& uclUID)
    // {
    //     mUclUID = uclUID;
    //     if (mUclUID == uclUID)
    //     {
    //         return Status(Status::Code::GOOD_ENTRY_REPLACED);
    //     }
    //     else
    //     {
    //         return Status(Status::Code::BAD_DEVICE_FAILURE);
    //     }
    // }

    // Status Alarm::SetUclPID(const std::string& uclPID)
    // {
    //     mUclPID = uclPID;
    //     if (mUclPID == uclPID)
    //     {
    //         return Status(Status::Code::GOOD_ENTRY_REPLACED);
    //     }
    //     else
    //     {
    //         return Status(Status::Code::BAD_DEVICE_FAILURE);
    //     }
    // }

    // Status Alarm::SetLclUID(const std::string& lclUID)
    // {
    //     mLclUID = lclUID;
    //     if (mLclUID == lclUID)
    //     {
    //         return Status(Status::Code::GOOD_ENTRY_REPLACED);
    //     }
    //     else
    //     {
    //         return Status(Status::Code::BAD_DEVICE_FAILURE);
    //     }
    // }

    // Status Alarm::SetLclPID(const std::string& lclPID)
    // {
    //     mLclPID = lclPID;
    //     if (mLclPID == lclPID)
    //     {
    //         return Status(Status::Code::GOOD_ENTRY_REPLACED);
    //     }
    //     else
    //     {
    //         return Status(Status::Code::BAD_DEVICE_FAILURE);
    //     }
    // }

    const bool& Alarm::HasUCL() const
    {
        return mIsUclSet;
    }

    const bool& Alarm::HasLCL() const
    {
        return mIsLclSet;
    }

    const bool& Alarm::HasCondition() const
    {
        return mHasCondition;
    }

    const std::string& Alarm::GetNodeID() const
    {
        return mNodeID;
    }

    const uint8_t& Alarm::GetAlarmType() const
    {
        return mAlarmType;
    }

    const double& Alarm::GetUCL() const
    {
        return mUCL;
    }

    const double& Alarm::GetLCL() const
    {
        return mLCL;
    }

    const std::vector<uint16_t>& Alarm::GetCondition() const
    {
        return mCondition;
    }

    // const std::string& Alarm::GetUclUID() const
    // {
    //     return mUclUID;
    // }

    // const std::string& Alarm::GetLclUID() const
    // {
    //     return mLclUID;
    // }

    // const std::string& Alarm::GetUclPID() const
    // {
    //     return mUclPID;
    // }

    // const std::string& Alarm::GetLclPID() const
    // {
    //     return mLclPID;
    // }
}}}


