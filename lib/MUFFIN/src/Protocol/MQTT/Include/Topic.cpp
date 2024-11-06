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
            
        #if defined(DEBUG)
            mLastWill               = "test/scautr/modlink/status/network/will";
            mJarvisRequest          = "mfm/" + macAddress;
            mJarvisResponse         = "mfm/resp/" + macAddress;
            mRemoteControlRequest   = "test/scautr/req/" + macAddress;
            mRemoteControlResponse  = "test/scautr/resp/" + macAddress;
            mDaqIntput              = "test/scautr/equipment/daq/input";
            mDaqOutput              = "test/scautr/equipment/daq/output";
            mDaqParam               = "test/scautr/equipment/param";
            mAlarm                  = "test/scautr/equipment/status/alarm";
            mError                  = "test/scautr/equipment/status/error";
            mOperation              = "test/scautr/equipment/status/operation";
            mUptime                 = "test/progix/dashboard/ut";
            mFinishedGoods          = "test/progix/dashboard/fg";
            mFotaConfig             = "fota/config";
            mFotaUpdate             = "fota/update/" + macAddress;
            mFotaStatus             = "fota/status";
        #else
            mLastWill               = "scautr/modlink/status/network/will";
            mJarvisRequest          = "mfm/" + macAddress;
            mJarvisResponse         = "mfm/resp/" + macAddress;
            mRemoteControlRequest   = "scautr/req/" + macAddress;
            mRemoteControlResponse  = "scautr/resp/" + macAddress;
            mDaqIntput              = "scautr/equipment/daq/input";
            mDaqOutput              = "scautr/equipment/daq/output";
            mDaqParam               = "scautr/equipment/param";
            mAlarm                  = "scautr/equipment/status/alarm";
            mError                  = "scautr/equipment/status/error";
            mOperation              = "scautr/equipment/status/operation";
            mUptime                 = "progix/dashboard/ut";
            mFinishedGoods          = "progix/dashboard/fg";
            mFotaConfig             = "fota/config";
            mFotaUpdate             = "fota/update/" + macAddress;
            mFotaStatus             = "fota/status";
        #endif
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
        case topic_e::DAQ_INPUT:
            return mDaqIntput.c_str();
        case topic_e::DAQ_OUTPUT:
            return mDaqOutput.c_str();
        case topic_e::DAQ_PARAM:
            return mDaqParam.c_str();
        case topic_e::ALARM:
            return mAlarm.c_str();
        case topic_e::ERROR:
            return mError.c_str();
        case topic_e::OPERATION:
            return mOperation.c_str();
        case topic_e::UPTIME:
            return mUptime.c_str();
        case topic_e::FINISHEDGOODS:
            return mFinishedGoods.c_str();
        case topic_e::FOTA_CONFIG:
            return mFotaConfig.c_str();
        case topic_e::FOTA_UPDATE:
            return mFotaUpdate.c_str();
        case topic_e::FOTA_STATUS:
            return mFotaStatus.c_str();
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
        else if (topicString == mDaqIntput)
        {
            return std::make_pair(true, topic_e::DAQ_INPUT);
        }
        else if (topicString == mDaqOutput)
        {
            return std::make_pair(true, topic_e::DAQ_OUTPUT);
        }
        else if (topicString == mDaqParam)
        {
            return std::make_pair(true, topic_e::DAQ_PARAM);
        }
        else if (topicString == mAlarm)
        {
            return std::make_pair(true, topic_e::ALARM);
        }
        else if (topicString == mError)
        {
            return std::make_pair(true, topic_e::ERROR);
        }
        else if (topicString == mOperation)
        {
            return std::make_pair(true, topic_e::OPERATION);
        }
        else if (topicString == mUptime)
        {
            return std::make_pair(true, topic_e::UPTIME);
        }
        else if (topicString == mFinishedGoods)
        {
            return std::make_pair(true, topic_e::FINISHEDGOODS);
        }
        else if (topicString == mFotaConfig)
        {
            return std::make_pair(true, topic_e::FOTA_CONFIG);
        }
        else if (topicString == mFotaUpdate)
        {
            return std::make_pair(true, topic_e::FOTA_UPDATE);
        }
        else if (topicString == mFotaStatus)
        {
            return std::make_pair(true, topic_e::FOTA_STATUS);
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
    std::string Topic::mDaqIntput;
    std::string Topic::mDaqOutput;
    std::string Topic::mDaqParam;
    std::string Topic::mAlarm;
    std::string Topic::mError;
    std::string Topic::mOperation;
    std::string Topic::mUptime;
    std::string Topic::mFinishedGoods;
    std::string Topic::mFotaConfig;
    std::string Topic::mFotaUpdate;
    std::string Topic::mFotaStatus;
}}