/**
 * @file CatMQTT.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 MQTT 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-09-11
 * @version 0.0.1
 * 
 * @todo 시간 제약으로 인해 Ver.0.0.1에서는 하기 함수를 구현하지 않기로 하였습니다.
 *       다음 버전에서는 미구현된 모든 함수를 구현하는 작업이 필용합니다.
 *       @li Init() 서브루틴에 미구현된 checker 함수 구현 및 적용
 *          @li checkPdpContext()
 *          @li checkSslContext()
 *          @li checkKeepAlive()
 *       @li Disconnect()
 *          @li disconnectBroker()
 *          @li closeSession()
 *       @li 이벤트 콜백 함수 구현
 *          @li onEventQMTSTAT()
 * 
 * @todo 다음과 같은 URC 코드의 처리를 구현해야 함(Processor 클래스에서!)
 *       @li +QIURC: "pdpdeact",1
 *       @li +QMTSTAT: 0,1
 * 
 * @todo Init(), Connect()와 같은 함수는 별도의 태스크로 만들어야 할 수도 있습니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <bitset>
#include <vector>

#include "Common/Status.h"
#include "Network/CatM1/CatM1.h"
#include "Network/TypeDefinitions.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Message.h"



namespace muffin { namespace mqtt {

    class CatMQTT
    {
    public:
        CatMQTT(CatMQTT const&) = delete;
        void operator=(CatMQTT const&) = delete;
        static CatMQTT* GetInstanceOrNULL(CatM1& catM1, BrokerInfo& broker, Message& lwt);
        static CatMQTT* GetInstanceOrNULL(CatM1& catM1, BrokerInfo& broker);
        static CatMQTT& GetInstance();
    private:
        CatMQTT(CatM1& catM1, BrokerInfo& broker, Message& lwt);
        CatMQTT(CatM1& catM1, BrokerInfo& broker);
        virtual ~CatMQTT();
    private:
        static CatMQTT* mInstance;

    public:
        Status Init(const network::lte::pdp_ctx_e pdp, const network::lte::ssl_ctx_e ssl);
        Status Connect();
        // Status Disconnect();
        Status IsConnected();
        Status Subscribe(const std::vector<Message>& messages);
        Status Unsubscribe(const std::vector<Message>& messages);
        Status Publish(const Message& message);
    private:
        Status setPdpContext(const network::lte::pdp_ctx_e pdp);
        Status setSslContext(const network::lte::ssl_ctx_e ssl);
        Status setVersion();
        Status setLastWill();
        Status setKeepAlive();
    private:
        // Status checkPdpContext();
        // Status checkSslContext();
        Status checkVersion();
        Status checkLastWill();
        // Status checkKeepAlive();
    private:
        Status openSession();
        Status connectBroker();
        // Status disconnectBroker();
        // Status closeSession();
    private:
        // void onEventQMTSTAT(const uint8_t socketID, const uint8_t errorCode);
        Status readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd);
        Status processCmeErrorCode(const std::string& rxd);
    private:
        CatM1& mCatM1;
        BrokerInfo mBrokerInfo;
        Message mMessageLWT;
        network::lte::pdp_ctx_e mContextPDP;
        network::lte::ssl_ctx_e mContextSSL;
    private:
        typedef enum CatMqttInitializationFlagEnum
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
        typedef enum CatMqttStateEnum
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