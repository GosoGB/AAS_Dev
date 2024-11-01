/**
 * @file Alarm.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Alarm 이벤트 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {

    class Alarm : public Base
    {
    public:
        Alarm();
        virtual ~Alarm() override;
    public:
        Alarm& operator=(const Alarm& obj);
        bool operator==(const Alarm& obj) const;
        bool operator!=(const Alarm& obj) const;    
    public:
        void SetNodeID(const std::string& nodeID);
        void SetType(const alarm_type_e type);
        void SetLCL(const float lcl);
        void SetLclUID(const std::string& lclUID);
        void SetLclAlarmUID(const std::string& lclAlarmUID);
        void SetUCL(const float ucl);
        void SetUclUID(const std::string& uclUID);
        void SetUclAlarmUID(const std::string& uclAlarmUID);
        void SetCondition(const std::vector<int16_t>& condition);
    public:
        std::pair<Status, std::string> GetNodeID() const;
        std::pair<Status, alarm_type_e> GetType() const;
        std::pair<Status, float> GetLCL() const;
        std::pair<Status, std::string> GetLclUID() const;
        std::pair<Status, std::string> GetLclAlarmUID() const;
        std::pair<Status, float> GetUCL() const;
        std::pair<Status, std::string> GetUclUID() const;
        std::pair<Status, std::string> GetUclAlarmUID() const;
        std::pair<Status, std::vector<int16_t>> GetCondition() const;
    private:
        bool mIsNodeIdSet      = false;
        bool mIsTypeSet        = false;
        bool mIsLclSet         = false;
        bool mIsLclUidSet      = false;
        bool mIsLclAlarmUidSet = false;
        bool mIsUclSet         = false;
        bool mIsUclUidSet      = false;
        bool mIsUclAlarmUidSet = false;
        bool mIsConditionSet   = false;
    private:
        std::string mNodeID;
        alarm_type_e mType;
        float mLCL = 0;
        std::string mLclUID;
        std::string mLclAlarmUID;
        float mUCL = 0;
        std::string mUclUID;
        std::string mUclAlarmUID;
        std::vector<int16_t> mCondition;
    public:
        float mPreviousLCL;
        float mPreviousUCL;
    };
}}}