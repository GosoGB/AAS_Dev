/**
 * @file CatM1.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * @details 현재는 Quectel 사의 BG96 칩셋을 사용한 LTE Cat.M1 모듈만을 대상으로 
 * 개발했습니다. 향후 향지에 따라서 별도의 칩셋을 사용한다면 추가 개발이 필요합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @todo 추후 버전 개발 시 FSM 수정 및 재적용이 필요함
 * @todo IPv6만 할당되고 있기 떄문에 인터페이스 수정이 필요함
 * @todo timeout 에러 발생 이후 RxD가 들어온다면 그건 또 어떻게 처리할지 결정해야 합니다.
 *       아마 버리는 게 합리적일 것 같은데 언제, 얼마나 timeout 이후에 들어올지 알 수 없기
 *       때문에 실제 처리는 조금 어렵지 않을까 생각합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <bitset>

#include "JARVIS/Config/Network/CatM1.h"
#include "Network/CatM1/Processor.h"
#include "Network/INetwork.h"



namespace muffin {

    class CatM1 : public INetwork
    {
    public:
        CatM1(CatM1 const&) = delete;
        void operator=(CatM1 const&) = delete;
        static CatM1* CreateInstanceOrNULL();
        static CatM1& GetInstance() noexcept;
    private:
        CatM1();
        virtual ~CatM1() override;
    private:
        static CatM1* mInstance;

    public:
        typedef enum class CatM1FiniteStateMachineEnum
            : int8_t
        {
            FAILED_TO_CONNECT            = -3,
            FAILED_TO_START              = -2,
            FAILED_TO_CONFIGURE          = -1,
            NOT_INITIALIZED_YET          =  0,
            SUCCEDDED_TO_INITIALIZE      =  1,
            SUCCEDDED_TO_CONFIGURE       =  2,
            SUCCEDDED_TO_START           =  3,
            SUCCEDDED_TO_CONNECT         =  4,
            SUCCEDDED_TO_GET_IP          =  5,
            CatM1_HAS_STOPPED            = 10,
            CatM1_DISCONNECTED           = 11
        } state_e;
    private:
        typedef enum LteModemInitializationFlagEnum
            : uint8_t
        {
            DIGITAL_PIN  = 0,
            SERIAL_PORT  = 1,
            URC_CALLBACK = 2,
            MODEM_BBP    = 3,
            FUNCTIONS    = 4,
            USIM_PIN     = 5,
            SMS_REPORT   = 6,
            APP_READY    = 7
        } init_flags_e;

        typedef enum LteModemConnectionFlagEnum
            : uint8_t
        {
            STATUS_PIN_GOOD   = 0,
            MODEM_AVAILABLE   = 1,
            PDP_CONFIGURED    = 2,
            PDP_ACTIVATED     = 3,
            GOT_OPERATOR      = 4,
            GOT_REGISTERED    = 5
        } conn_flags_e;

    public:
        virtual Status Init() override;
        virtual Status Config(jarvis::config::Base* config) override;
        virtual Status Connect() override;
        virtual Status Disconnect() override;
        virtual Status Reconnect() override;
        virtual bool IsConnected() const override;
        virtual IPAddress GetIPv4() const override;
        virtual jarvis::config::Base* GetConfig() override;
        std::pair<bool, jarvis::config::CatM1> RetrieveConfig() const;
        state_e GetState() const;
        Status SyncWithNTP();
        void KillUrcTask(bool forOTA);
    public:
        virtual std::pair<Status, size_t> TakeMutex() override;
        virtual Status ReleaseMutex() override;
        Status Execute(const std::string& command, const size_t mutexHandle);
        size_t GetAvailableBytes();
        int16_t Read();
        std::string ReadBetweenPatterns(const std::string& patternBegin, const std::string& patternEnd);
    private:
        Status isModemAvailable();
        Status checkOperator();
        Status checkRegistration();
        Status configurePdpContext();
        Status activatePdpContext();
        void getPdpContext();
    private:
        void resetModule();
        void onEventRDY();
        void onEventCFUN();
        void onEventCPIN(const std::string& state);
        void onEventQIND();
        void onEventAPPRDY();
        // void onEventQMTRECV();
        void checkCatM1Started();
        static void onEventPinStatusFalling(void* pvParameter, uint32_t ulParameter);
        static void IRAM_ATTR handlePinStatusISR();

    private:
        size_t mMutexHandle = 0;
        SemaphoreHandle_t xSemaphore;
        Processor mProcessor;
        std::pair<bool, jarvis::config::CatM1> mConfig;
        static state_e mState;
        static std::bitset<8> mInitFlags;
        static std::bitset<6> mConnFlags;
    #if defined(MODLINK_L)
        const uint8_t mPinStatus = 19;
        const uint8_t mPinReset  = 21;
    #elif defined(VOLA_T10M)
        const uint8_t mPinStatus = 36;
        const uint8_t mPinReset  = 33;
    #else
        const uint8_t mPinStatus = 33;
        const uint8_t mPinReset  = 14;
    #endif
        static const uint32_t mDebounceMillis = 7 * 1000; // LTE 모뎀 평균 부팅시간
        static uint32_t mLastInterruptMillis;
    };
}