/**
 * @file Topic.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 토픽 클래스를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Topic.h"



namespace muffin { namespace mqtt {

#if defined(DEBUG)
    bool gIsTopicCreated = false;
#endif

    bool Topic::CreateTopic(const std::string& macAddress)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) Topic(macAddress);
            if (mInstance == nullptr)
            {
                return false;
            }
            
            mLastWill               = "scautr/modlink/status/network/will";
            mJarvisRequest          = "mfm/" + macAddress;
            mJarvisResponse         = "mfm/resp/" + macAddress;
            mRemoteControlRequest   = "scautr/req/" + macAddress;
            mRemoteControlResponse  = "scautr/resp/" + macAddress;
        }

        return true;

    #if defined(DEBUG)
        ASSERT((gIsTopicCreated == false), "DO NOT CREATE TOPIC TWICE UNLESS YOU HAVE TO");
        gIsTopicCreated = true;
    #endif

    }
    
    Topic::Topic(const std::string& macAddress)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Topic::~Topic()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    const char* Topic::ToString(const topic_e topicCode)
    {
        switch (topicCode)
        {
        case topic_e::LAST_WILL:
            return mLastWill.c_str();
        case topic_e::JARVIS_REQUEST:
            return mJarvisRequest.c_str();
        case topic_e::JARVIS_RESPONSE:
            return mJarvisResponse.c_str();
        case topic_e::REMOTE_CONTROL_REQUEST:
            return mRemoteControlRequest.c_str();
        case topic_e::REMOTE_CONTROL_RESPONSE:
            return mRemoteControlResponse.c_str();
        default:
            ASSERT(false, "UNDEFINED TOPIC CODE: %u", static_cast<uint8_t>(topicCode));
            return nullptr;
        }
    }

    std::pair<bool, topic_e> Topic::ToCode(const std::string& topicString)
    {
        if (topicString == mJarvisRequest)
        {
            return std::make_pair(true, topic_e::JARVIS_REQUEST);
        }
        else if (topicString == mJarvisResponse)
        {
            return std::make_pair(true, topic_e::JARVIS_RESPONSE);
        }
        else if (topicString == mRemoteControlRequest)
        {
            return std::make_pair(true, topic_e::REMOTE_CONTROL_REQUEST);
        }
        else if (topicString == mLastWill)
        {
            return std::make_pair(true, topic_e::LAST_WILL);
        }
        else
        {
            return std::make_pair(false, topic_e::LAST_WILL);
        }
    }


    Topic* Topic::mInstance = nullptr;
    std::string Topic::mLastWill;
    std::string Topic::mJarvisRequest;
    std::string Topic::mJarvisResponse;
    std::string Topic::mRemoteControlRequest;
    std::string Topic::mRemoteControlResponse;
}}