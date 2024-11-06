/**
 * @file Topic.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜의 토픽 클래스를 선언합니다.
 * 
 * @date 2024-10-17
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once


#include <string>
#include <sys/_stdint.h>

#include "TypeDefinitions.h"



namespace muffin { namespace mqtt {

    class Topic
    {
    public:
        Topic(Topic const&) = delete;
        void operator=(Topic const&) = delete;
        static bool CreateTopic(const std::string& macAddress);
    private:
        Topic(const std::string& macAddress);
        virtual ~Topic();
    private:
        static Topic* mInstance;

    public:
        static const char* ToString(const topic_e topicCode);
        static std::pair<bool, topic_e> ToCode(const std::string& topicString);
    private:
        static std::string mLastWill;
        static std::string mJarvisRequest;
        static std::string mJarvisResponse;
        static std::string mRemoteControlRequest;
        static std::string mRemoteControlResponse;
        static std::string mDaqIntput;
        static std::string mDaqOutput;
        static std::string mDaqParam;
        static std::string mAlarm;
        static std::string mError;
        static std::string mOperation;
        static std::string mUptime;
        static std::string mFinishedGoods;
        static std::string mFotaConfig;
        static std::string mFotaUpdate;
    };
}}