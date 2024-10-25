/**
 * @file BaseEventType.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 발생시키는 이벤트에 대한 기본 클래스를 선언합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "BaseEventType.h"



namespace muffin { namespace im {

    BaseEventType::BaseEventType()
        : mEventID(generateEventID())
        , mReceiveTime(GetTimestampInMillis())
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    BaseEventType::BaseEventType(const std::string& sourceNodeID)
        : mEventID(generateEventID())
        , mSourceNodeID(sourceNodeID)
        , mReceiveTime(GetTimestampInMillis())
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    BaseEventType::BaseEventType(const std::string& sourceNodeID, const std::string& eventMessage)
        : mEventID(generateEventID())
        , mSourceNodeID(sourceNodeID)
        , mEventMessage(eventMessage)
        , mReceiveTime(GetTimestampInMillis())
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    BaseEventType::~BaseEventType()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void BaseEventType::SetSourceNodeID(const std::string& nodeID)
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
    }

    void BaseEventType::SetEventMessage(const std::string& message)
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
    }

    std::string BaseEventType::GetEventID() const
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return std::string();
    }

    std::string BaseEventType::GetEventMessage() const
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return std::string();
    }

    std::string BaseEventType::GetSourceNodeID() const
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return std::string();
    }

    uint64_t BaseEventType::GetReceiveTime() const
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return 0;
    }
}}