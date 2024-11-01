/**
 * @file Alarm.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Alarm 이벤트 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#if defined(DEBUG)
    #include <regex>
#endif

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Alarm.h"



namespace muffin { namespace jarvis { namespace config {

    Alarm::Alarm()
        : Base(cfg_key_e::ALARM)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Alarm::~Alarm()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Alarm& Alarm::operator=(const Alarm& obj)
    {
        if (this != &obj)
        {
            mNodeID     =  obj.mNodeID;
            mType       =  obj.mType;
            mLCL        =  obj.mLCL;
            mLclUID     =  obj.mLclUID;
            mUCL        =  obj.mUCL;
            mUclUID     =  obj.mUclUID;
            mCondition  =  obj.mCondition;
        }
        
        return *this;
    }

    bool Alarm::operator==(const Alarm& obj) const
    {
       return (
            mNodeID     ==  obj.mNodeID     &&
            mType       ==  obj.mType       &&
            mLCL        ==  obj.mLCL        &&
            mLclUID     ==  obj.mLclUID     && 
            mUCL        ==  obj.mUCL        &&
            mUclUID     ==  obj.mUclUID     && 
            mCondition  ==  obj.mCondition
        );
    }

    bool Alarm::operator!=(const Alarm& obj) const
    {
        return !(*this == obj);
    }

    void Alarm::SetNodeID(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeID = nodeID;
        mIsNodeIdSet = true;
    }

    void Alarm::SetType(const alarm_type_e type)
    {
        mType = type;
        mIsTypeSet = true;
    }

    void Alarm::SetLCL(const float lcl)
    {
        mLCL = lcl;
        mIsLclSet = true;
    }

    void Alarm::SetLclUID(const std::string& lclUID)
    {
        ASSERT(
            (
                [&]()
                {
                    const std::regex pattern("^P\\d{3}$");
                    return std::regex_match(lclUID, pattern);
                }()
            ), "INVALID LCL UID: %s", lclUID.c_str()
        );

        mLclUID = lclUID;
        mIsLclUidSet = true;
    }

    void Alarm::SetLclAlarmUID(const std::string& lclAlarmUID)
    {
        ASSERT(
            (
                [&]()
                {
                    const std::regex pattern("^A\\d{3}$");
                    return std::regex_match(lclAlarmUID, pattern);
                }()
            ), "INVALID LCL UID: %s", lclAlarmUID.c_str()
        );

        mLclAlarmUID = lclAlarmUID;
        mIsLclAlarmUidSet = true;
    }

    void Alarm::SetUCL(const float ucl)
    {
        mUCL = ucl;
        mIsUclSet = true;
    }

    void Alarm::SetUclUID(const std::string& uclUID)
    {
        ASSERT(
            (
                [&]()
                {
                    const std::regex pattern("^P\\d{3}$");
                    return std::regex_match(uclUID, pattern);
                }()
            ), "INVALID UCL UID: %s", uclUID.c_str()
        );

        mUclUID = uclUID;
        mIsUclUidSet = true;
    }

    void Alarm::SetUclAlarmUID(const std::string& uclAlarmUID)
    {
        ASSERT(
            (
                [&]()
                {
                    const std::regex pattern("^A\\d{3}$");
                    return std::regex_match(uclAlarmUID, pattern);
                }()
            ), "INVALID LCL ALARM UID: %s", uclAlarmUID.c_str()
        );

        mUclAlarmUID = uclAlarmUID;
        mIsUclAlarmUidSet = true;
    }

    void Alarm::SetCondition(const std::vector<int16_t>& condition)
    {
        ASSERT((condition.size() != 0), "INPUT PARAMETER <condition> CANNOT BE AN EMPTY VECTOR");

        mCondition = condition;
        mIsConditionSet = true;
    }

    std::pair<Status, std::string> Alarm::GetNodeID() const
    {
        if (mIsNodeIdSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeID);
        }
    }

    std::pair<Status, alarm_type_e> Alarm::GetType() const
    {
        if (mIsTypeSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mType);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mType);
        }
    }

    std::pair<Status, float> Alarm::GetLCL() const
    {
        if (mIsLclSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mLCL);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mLCL);
        }
    }

    std::pair<Status, std::string> Alarm::GetLclUID() const
    {
        if (mIsLclUidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mLclUID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mLclUID);
        }
    }

    std::pair<Status, std::string> Alarm::GetLclAlarmUID() const
    {
        if (mIsLclAlarmUidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mLclAlarmUID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mLclAlarmUID);
        }
    }

    std::pair<Status, float> Alarm::GetUCL() const
    {
        if (mIsUclSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mUCL);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mUCL);
        }
    }

    std::pair<Status, std::string> Alarm::GetUclUID() const
    {
        if (mIsUclUidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mUclUID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mUclUID);
        }
    }

    std::pair<Status, std::string> Alarm::GetUclAlarmUID() const
    {
        if (mIsUclAlarmUidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mUclAlarmUID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mUclAlarmUID);
        }
    }

    std::pair<Status, std::vector<int16_t>> Alarm::GetCondition() const
    {
        if (mIsConditionSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mCondition);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mCondition);
        }
    }

}}}
