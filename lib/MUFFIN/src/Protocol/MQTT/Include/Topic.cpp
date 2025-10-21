/**
 * @file Topic.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 토픽 클래스를 정의합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <string.h>

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Topic.h"



namespace muffin { namespace mqtt {

    void Topic::Init()
    {
        snprintf(
            mFotaUpdate,
            sizeof(mFotaUpdate),
            "fota/update/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mJarvisRequest,
            sizeof(mJarvisRequest),
            "mfm/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mJarvisResponse,
            sizeof(mJarvisResponse),
            "mfm/resp/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mJarvisInterfaceRequest,
            sizeof(mJarvisInterfaceRequest),
            "mfm/if/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mJarvisInterfaceResponse,
            sizeof(mJarvisInterfaceResponse),
            "mfm/if/resp/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mRemoteControlRequest,
            sizeof(mRemoteControlRequest),
            "scautr/req/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mRemoteControlResponse,
            sizeof(mRemoteControlResponse),
            "scautr/resp/%s",
            macAddress.GetEthernet()
        );

        snprintf(
            mAAS_OPERATIONALDATA_RTM,
            sizeof(mAAS_OPERATIONALDATA_RTM),
            "submodels/%s/submodel-elements/RealTimeMonitoring",
            SM_ID_B64_OPERATIONAL_DATA
        );

        snprintf(
            mAAS_OPERATIONALDATA_JP,
            sizeof(mAAS_OPERATIONALDATA_JP),
            "submodels/%s/submodel-elements/JobProgress",
            SM_ID_B64_OPERATIONAL_DATA
        );

        snprintf(
            mAAS_CONFIGURATION,
            sizeof(mAAS_CONFIGURATION),
            "submodels/%s/submodel-elements/BasicConfiguration",
            SM_ID_B64_CONFIGURATION
        );
    }
     
    const char* Topic::ToString(const topic_e topicCode)
    {
        switch (topicCode)
        {
        case topic_e::LAST_WILL:
            return mLastWill;

        case topic_e::JARVIS_REQUEST:
            return mJarvisRequest;
            
        case topic_e::JARVIS_RESPONSE:
            return mJarvisResponse;

        case topic_e::JARVIS_INTERFACE_REQUEST:
            return mJarvisInterfaceRequest;
            
        case topic_e::JARVIS_INTERFACE_RESPONSE:
            return mJarvisInterfaceResponse;

        case topic_e::JARVIS_STATUS:
            return mJarvisStatus;
            
        case topic_e::REMOTE_CONTROL_REQUEST:
            return mRemoteControlRequest;
            
        case topic_e::REMOTE_CONTROL_RESPONSE:
            return mRemoteControlResponse;
            
        case topic_e::DAQ_INPUT:
            return mDaqIntput;
            
        case topic_e::DAQ_OUTPUT:
            return mDaqOutput;
            
        case topic_e::DAQ_PARAM:
            return mDaqParam;
            
        case topic_e::ALARM:
            return mAlarm;
            
        case topic_e::ERROR:
            return mError;
            
        case topic_e::PUSH:
            return mPush;
            
        case topic_e::OPERATION:
            return mOperation;
            
        case topic_e::UPTIME:
            return mUptime;
            
        case topic_e::FINISHEDGOODS:
            return mFinishedGoods;
            
        case topic_e::FOTA_CONFIG:
            return mFotaConfig;
            
        case topic_e::FOTA_UPDATE:
            return mFotaUpdate;
            
        case topic_e::FOTA_STATUS:
            return mFotaStatus;

        case topic_e::AAS_OPERATIONALDATA_RTM:
            return mAAS_OPERATIONALDATA_RTM;

        case topic_e::AAS_OPERATIONALDATA_JP:
            return mAAS_OPERATIONALDATA_JP;

        case topic_e::AAS_CONFIGURATION:
            return mAAS_CONFIGURATION;
            
        default:
            ASSERT(false, "UNDEFINED TOPIC CODE: %u", static_cast<uint8_t>(topicCode));
            return nullptr;
        }
    }

    std::pair<bool, topic_e> Topic::ToCode(const char* topicString)
    {
        if (strcmp(topicString, mJarvisRequest) == 0)
        {
            return std::make_pair(true, topic_e::JARVIS_REQUEST);
        }
        else if (strcmp(topicString, mJarvisResponse) == 0)
        {
            return std::make_pair(true, topic_e::JARVIS_RESPONSE);
        }
        else if (strcmp(topicString, mJarvisInterfaceRequest) == 0)
        {
            return std::make_pair(true, topic_e::JARVIS_INTERFACE_REQUEST);
        }
        else if (strcmp(topicString, mJarvisInterfaceResponse) == 0)
        {
            return std::make_pair(true, topic_e::JARVIS_INTERFACE_RESPONSE);
        }
        else if (strcmp(topicString, mJarvisStatus) == 0)
        {
            return std::make_pair(true, topic_e::JARVIS_STATUS);
        }
        else if (strcmp(topicString, mRemoteControlRequest) == 0)
        {
            return std::make_pair(true, topic_e::REMOTE_CONTROL_REQUEST);
        }
        else if (strcmp(topicString, mLastWill) == 0)
        {
            return std::make_pair(true, topic_e::LAST_WILL);
        }
        else if (strcmp(topicString, mDaqIntput) == 0)
        {
            return std::make_pair(true, topic_e::DAQ_INPUT);
        }
        else if (strcmp(topicString, mDaqOutput) == 0)
        {
            return std::make_pair(true, topic_e::DAQ_OUTPUT);
        }
        else if (strcmp(topicString, mDaqParam) == 0)
        {
            return std::make_pair(true, topic_e::DAQ_PARAM);
        }
        else if (strcmp(topicString, mAlarm) == 0)
        {
            return std::make_pair(true, topic_e::ALARM);
        }
        else if (strcmp(topicString, mError) == 0)
        {
            return std::make_pair(true, topic_e::ERROR);
        }
        else if (strcmp(topicString, mPush) == 0)
        {
            return std::make_pair(true, topic_e::PUSH);
        }
        else if (strcmp(topicString, mOperation) == 0)
        {
            return std::make_pair(true, topic_e::OPERATION);
        }
        else if (strcmp(topicString, mUptime) == 0)
        {
            return std::make_pair(true, topic_e::UPTIME);
        }
        else if (strcmp(topicString, mFinishedGoods) == 0)
        {
            return std::make_pair(true, topic_e::FINISHEDGOODS);
        }
        else if (strcmp(topicString, mFotaConfig) == 0)
        {
            return std::make_pair(true, topic_e::FOTA_CONFIG);
        }
        else if (strcmp(topicString, mFotaUpdate) == 0)
        {
            return std::make_pair(true, topic_e::FOTA_UPDATE);
        }
        else if (strcmp(topicString, mFotaStatus) == 0)
        {
            return std::make_pair(true, topic_e::FOTA_STATUS);
        }
        else
        {
            return std::make_pair(false, topic_e::LAST_WILL);
        }
    }


    Topic topic;
}}