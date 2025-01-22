/**
 * @file LwipMQTT.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief FreeRTOS TCP/IP 스택인 LwIP를 사용하는 MQTT 프로토콜 클래스를 선언합니다.
 * 
 * @date 2025-01-22
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <vector>
#include <WiFiClientSecure.h>

#include "Common/Status.h"
#include "Protocol/MQTT/IMQTT.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/LwipMQTT/PubSubClient.h"



namespace muffin { namespace mqtt {

    class LwipMQTT : public IMQTT
    {
    public:
        LwipMQTT(BrokerInfo& broker, Message& lwt) : mBrokerInfo(std::move(broker)), mMessageLWT(std::move(lwt)) {}
        virtual ~LwipMQTT() override {}
    public:
        Status Init();
        virtual Status Connect(const size_t mutexHandle) override;
        virtual Status Disconnect(const size_t mutexHandle) override;
        virtual Status IsConnected() override;
        virtual Status Subscribe(const size_t mutexHandle, const std::vector<Message>& messages) override;
        virtual Status Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages) override;
        virtual Status Publish(const size_t mutexHandle, const Message& message) override;
        virtual INetwork* RetrieveNIC() override;
    private:
        const char* getState();
        static void vTimerCallback(TimerHandle_t xTimer);
        void callback(char* topic, byte * payload, unsigned int length);
    private:
        static PubSubClient mClient;
        WiFiClientSecure mNIC;
        TimerHandle_t xTimer = NULL;
    private:
        const BrokerInfo mBrokerInfo;
        const Message mMessageLWT;
        const uint16_t BUFFER_SIZE = 512;
        const uint8_t KEEP_ALIVE  =  10;
    };
}}