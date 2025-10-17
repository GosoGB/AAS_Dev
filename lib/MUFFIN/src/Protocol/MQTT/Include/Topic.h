/**
 * @file Topic.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 토픽 클래스를 선언합니다.
 * 
 * @date 2024-10-17
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>

#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    class Topic
    {
    public:
        Topic() {}
        virtual ~Topic() {}
    public:
        void Init();
        const char* ToString(const topic_e topicCode);
        std::pair<bool, topic_e> ToCode(const char* topicString);
    private:
        const char* mLastWill        = "mfm/status/lwt";
        const char* mDaqIntput       = "scautr/equipment/daq/input";
        const char* mDaqOutput       = "scautr/equipment/daq/output";
        const char* mDaqParam        = "scautr/equipment/param";
        const char* mAlarm           = "scautr/equipment/status/alarm";
        const char* mError           = "scautr/equipment/status/error";
        const char* mPush            = "progix/push";
        const char* mOperation       = "scautr/equipment/status/operation";
        const char* mUptime          = "progix/dashboard/ut";
        const char* mFinishedGoods   = "progix/dashboard/fg";
        const char* mFotaConfig      = "fota/config";
        const char* mFotaStatus      = "fota/status";
        const char* mJarvisStatus    = "mfm/status";
    private:
        char mFotaUpdate[25] = {'\0'};
        char mJarvisRequest[17] = {'\0'};
        char mJarvisResponse[22] = {'\0'};
        char mJarvisInterfaceRequest[20] = {'\0'};
        char mJarvisInterfaceResponse[25] = {'\0'};
        char mRemoteControlRequest[24] = {'\0'};
        char mRemoteControlResponse[25] = {'\0'};
        char mAAS_MQTT[256] = {'\0'};
    };


    extern Topic topic;
}}