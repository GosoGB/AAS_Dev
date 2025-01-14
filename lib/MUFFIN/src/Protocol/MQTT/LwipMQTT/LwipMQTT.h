/**
 * @file LwipMQTT.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-01-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */




#pragma once

#include <WiFiClientSecure.h>
#include <bitset>
#include <vector>

#include "Common/Status.h"
#include "Protocol/MQTT/LwipMQTT/PubSubClient.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/IMQTT.h"



namespace muffin { namespace mqtt {

    class LwipMQTT : public IMQTT
    {
    public:
        LwipMQTT() = delete;
        void operator=(LwipMQTT const&) = delete;
        static LwipMQTT* CreateInstanceOrNULL( BrokerInfo& broker, Message& lwt);
        static LwipMQTT* CreateInstanceOrNULL( BrokerInfo& broker);
        static LwipMQTT& GetInstance();
    private:
        LwipMQTT( BrokerInfo& broker, Message& lwt);
        LwipMQTT( BrokerInfo& broker);
        virtual ~LwipMQTT() override;
    private:
        static LwipMQTT* mInstance;

    public:
        Status Init();
        virtual Status Connect(const size_t mutexHandle) override;
        virtual Status Disconnect(const size_t mutexHandle) override;
        virtual Status IsConnected() override;
        virtual Status Subscribe(const size_t mutexHandle, const std::vector<Message>& messages) override;
        virtual Status Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages) override;
        virtual Status Publish(const size_t mutexHandle, const Message& message) override;
    public:
        void OnEventReset();
        std::pair<Status, size_t> TakeMutex();
        Status ReleaseMutex();
    private:
        Status setLastWill(const size_t mutexHandle);
        Status setKeepAlive(const size_t mutexHandle);
        Status connectBroker(const size_t mutexHandle);
        Status disconnectBroker(const size_t mutexHandle);
        void callback(char* topic, byte * payload, unsigned int length);

    private:
        bool mIsFirstConnect = false;
        WiFiClientSecure mLwipSecureClient;
        PubSubClient mPubSubClient;
        BrokerInfo mBrokerInfo;
        Message mMessageLWT;
        size_t mMutexHandle = 0;
        SemaphoreHandle_t xSemaphore;
    private:
        typedef enum LwipMqttInitializationFlagEnum
            : uint8_t
        {
            INITIALIZED_PDP   = 0, // Set if PDP context is initialized, reset otherwise
            INITIALIZED_SSL   = 1, // Set if SSL context is initialized, reset otherwise
            INITIALIZED_VSN   = 2, // Set if protocol version is initialized, reset otherwise
            INITIALIZED_LWT   = 3, // Set if last will and testament is initialized, reset otherwise
            INITIALIZED_KAT   = 4, // Set if keep alive time is initialized, reset otherwise
            INITIALIZED_ALL   = 5, // Set if initialization succeded, reset otherwise
            ENABLE_LWT_MSG    = 6, // Set if LWT should be configured, reset otherwise
        } init_flag_e;
        std::bitset<7> mInitFlags;
        typedef enum LwipMqttStateEnum
            : int8_t
        {
            CONNECT_FAILED   = -2,
            INIT_FAILED      = -1,
            CONSTRUCTED      =  0,
            INITIALIZED      =  1,
            CONNECTED        =  2,
            DISCONNECTED     =  3
        } state_e;
        state_e mState;
    };
}}