/**
 * @file Alarm.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {


    class Alarm : public Base
    {
    public:
        Alarm(const std::string& key);
        virtual ~Alarm() override;
    public:
        Alarm& operator=(const Alarm& obj);
        bool operator==(const Alarm& obj) const;
        bool operator!=(const Alarm& obj) const;
    
    public:
        Status SetNodeID(const std::string& nodeID);
        Status SetAlarmType(const uint8_t& type);
        Status SetUCL(const double& ucl);
        Status SetLCL(const double& lcl);
        Status SetCondition(const std::vector<uint16_t>& condition);
    
    // public:
    //     Status SetUclUID(const std::string& uclUID);
    //     Status SetLclUID(const std::string& lclUID);
    //     Status SetUclPID(const std::string& uclPID);
    //     Status SetLclPID(const std::string& lclPID);   

    public:
        const bool& HasUCL() const;
        const bool& HasLCL() const;
        const bool& HasCondition() const;

    public:
        const std::string& GetNodeID() const;
        const uint8_t& GetAlarmType() const;
        const double& GetUCL() const;
        const double& GetLCL() const;
        const std::vector<uint16_t>& GetCondition() const;
 
    // public: 
    //     const std::string& GetUclUID() const;
    //     const std::string& GetLclUID() const;
    //     const std::string& GetUclPID() const;
    //     const std::string& GetLclPID() const;

    private:
        bool mHasCondition = false;
        bool mHasNodeID = false;
        bool mIsUclSet = false;
        bool mIsLclSet = false;

    private:
        std::string mNodeID;
        std::vector<uint16_t> mCondition;
        uint8_t mAlarmType;
        double mUCL = 0;
        double mLCL = 0;
        // std::string mLclUID;
        // std::string mUclUID;
        // std::string mLclPID;
        // std::string mUclPID;
    };
}}}
