/**
 * @file Processor.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈과의 모든 통신을 처리하는 클래스를 선언합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @todo 시리얼 포트에 사용되는 핀(pin)을 사용 중이지 않은지 확인하는 클래스가 필요함
 *       MODLINK-L은 다른 링크를 사용할 수 없기 때문에 상관 없지만 MODLINK-B 또는 
 *       MODLINK-T2에서는 다른 링크가 핀을 사용함으로써 통신이 망가질 가능성이 있음
 * @todo SetBaudRate(), SetTimeout() 함수는 현재 사용하지 않기 때문에 시간 관계 상 만들지 않았습니다.
 * @todo LTE Cat.M1 모뎀 부팅 시 들어오는 URC 중 "QUSIM", "QMTRECV", "QMTSTAT" 메시지 처리 구현해야 함
 *       특히, QUSIM은 USIM 카드가 사용 가능한 상태인지와 같은 정보를 나타내는 것으로 보이는데 이에 대한 
 *       내용이 BG96 관련 문서에는 없어서 정확한 내용을 확인해야 합니다.
 * @todo mRxBuffer에 있는 데이터가 오랫동안 비워지지 않을 경우에 대한 처리를 구현해야 함
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <bitset>
#include <HardwareSerial.h>

#include "Common/DataStructure/CircularBuffer.h"
#include "Common/Status.h"



namespace muffin {

    class Processor
    {
    public:
        Processor();
        virtual ~Processor();
    private:
        HardwareSerial mSerial;
        const uint16_t mRxBufferSize;
        CircularBuffer mRxBuffer;
        const uint16_t mTimeoutMillis;
    #ifdef MODLINK_L
        const uint8_t mPinTxD =  5;
        const uint8_t mPinRxD = 18;
    #elif VOLA_T10M
        const uint8_t mPinTxD = 16;
        const uint8_t mPinRxD = 17;
    #else
        const uint8_t mPinTxD =  4;
        const uint8_t mPinRxD =  5;
    #endif
        typedef enum class BG96BaudRateEnum
            : uint32_t
        {
            BDR_9600        =    9600,
            BDR_19200       =   19200,
            BDR_38400       =   38400,
            BDR_57600       =   57600,
            BDR_115200      =  115200,  // default value
            BDR_230400      =  230400,
            BDR_460800      =  460800,
            BDR_921600      =  921600   // highest speed
        } baudrate_e;
        baudrate_e mBaudRate;

    public:
        Status Init();
        Status SetBaudRate(const baudrate_e baudRate);
        Status SetTimeout(const uint16_t timeout);
        uint32_t GetBaudRate() const;
        uint32_t GetTimeout() const;
    public:
        Status Write(const std::string& command);
        int16_t Read();
        std::string ReadBetweenPatterns(const std::string& patternBegin, const std::string& patternEnd);
        size_t GetAvailableBytes();
        void StopUrcHandleTask(bool forOTA);
    private:
        void stopUrcHandleTask();
        void implementUrcHandleTask();
        static void wrapUrcHandleTask(void* pvParameters);
    private:
        void parseRDY();
        void parseCFUN();
        void parseCPIN();
        void parseQIND();
        void parseAPPRDY();
        void parseQMTRECV();
        // void parseQMTSTAT();
    private:
        typedef enum LteModuleProcessorInitializationFlagEnum
            : uint8_t
        {
            SERIAL_PORT_INITIALIZED  = 0,
            TASK_SEMAPHORE_CREATED   = 1,
            PROCESSOR_TASK_CREATED   = 2
        } init_flags_e;
        std::bitset<4> mInitFlags;
        TaskHandle_t xHandle;
        SemaphoreHandle_t xSemaphore;
        const uint8_t mTaskInterval;
        bool mHasOTA = false;

    public:
        void RegisterCallbackRDY(const std::function<void()>& cb);
        void RegisterCallbackCFUN(const std::function<void()>& cb);
        void RegisterCallbackCPIN(const std::function<void(const std::string&)>& cb);
        void RegisterCallbackQIND(const std::function<void()>& cb);
        void RegisterCallbackAPPRDY(const std::function<void()>& cb);
        // void RegisterCallbackQMTRECV(const std::function<void()>& cb);
        // void RegisterCallbackQMTSTAT(const std::function<void(uint8_t, uint8_t)>& cb);
    private:
        void triggerCallbackRDY();
        void triggerCallbackCFUN();
        void triggerCallbackCPIN(const std::string& state);
        void triggerCallbackQIND();
        void triggerCallbackAPPRDY();
        // void triggerCallbackQMTRECV();
        // void triggerCallbackQMTSTAT(const uint8_t socketID, const uint8_t errorCode);
    private:
        std::function<void()> mCallbackRDY;
        std::function<void()> mCallbackCFUN;
        std::function<void(const std::string&)> mCallbackCPIN;
        std::function<void()> mCallbackQIND;
        std::function<void()> mCallbackAPPRDY;
        // std::function<void()> mCallbackQMTRECV;
        // std::function<void(uint8_t, uint8_t)> mCallbackQMTSTAT;
    };
}