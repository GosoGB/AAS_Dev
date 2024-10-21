/**
 * @file Event.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 발생시키는 이벤트에 대한 기본 클래스를 선언합니다.
 * 
 * @date 2024-10-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Status.h"



namespace muffin { namespace im {

    class Event
    {
    public:
        Event();
        Event(const std::string& sourceNodeID);
        Event(const std::string& sourceNodeID, const std::string& eventMessage);
        virtual ~Event();
    public:
        void SetSourceNodeID(const std::string& nodeID);
        void SetEventMessage(const std::string& message);
    public:
        std::string GetEventID() const;
        std::string GetEventMessage() const;
        std::string GetSourceNodeID() const;
        uint64_t GetReceiveTime() const;
    private:
        std::string generateEventID() const;
    private:
        std::string mEventID;
        std::string mSourceNodeID;
        std::string mEventMessage;
        uint64_t mReceiveTime;
    };
}}