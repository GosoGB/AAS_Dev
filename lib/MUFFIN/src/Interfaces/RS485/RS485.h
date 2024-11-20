/**
 * @file RS485.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief RS-485/422 시리얼 통신 인터페이스 클래스를 선언합니다.
 * @note 본 클래스는 ANANLOG DEVICES 사의 MAX485 트랜스시버를 사용합니다.
 * 
 * @date 2024-10-21
 * @version 1.0.0
 * 
 * @note 하드웨어 개발팀의 김병우 수석께 확인한 결과 MODLINK-L, MODLINK-ML10 모델의 경우
 *       MAX485 칩셋의 DE/RE 핀은 ESP32에 연결되어 있지 않습니다. MAX485 칩셋 실장 시 
 *       회로 상에서 TxD 신호에 따라 DE/RE 핀이 자동으로 연동되게 설계되어 있습니다.
 * 
 * @note ATmega2560의 경우에는 링크에 따라 DE/RE 핀을 수동으로 제어해야 하는 RS-485 링크가
 *       있었지만 현재는 단종된 상태로 ESP32와 동일하게 펌웨어에서 제어할 수 없습니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <HardwareSerial.h>
#include <Stream.h>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "Jarvis/Config/Interfaces/Rs485.h"



namespace muffin {

    class RS485 : public Stream
    {
    public:
        RS485(RS485 const&) = delete;
        void operator=(RS485 const&) = delete;
        static RS485* GetInstanceOrNull();
        static RS485& GetInstance();
        operator bool();
    private:
        RS485();
        virtual ~RS485();
    private:
        static RS485* mInstance;

    public:
        void Config(jarvis::config::Rs485* config);
    public:
        void begin();
        void begin(const uint32_t baudrate);
        void begin(const uint32_t baudrate, const uint32_t config);
        void begin(const uint32_t baudrate, const uint16_t predelay, const uint16_t postdelay);
        void begin(const uint32_t baudrate, const uint32_t config, const uint16_t predelay, const uint16_t postdelay);
        void end();
    public:
        int32_t available();
        int32_t peek();
        int32_t read(void);
        size_t write(const uint8_t byte);
        size_t write(const uint8_t* buffer, size_t size);
        size_t write(const char* buffer, size_t size);
    public:
        void beginTransmission();
        void endTransmission();
        void receive();
        void noReceive();
        void sendBreak(const uint32_t duration);
        void sendBreakMicroseconds(const uint32_t duration);
    private:
        bool mIsTransmissionBegun = false;
        bool mIsConfigured = false;
    private:
        uint16_t mPreDelay  = 50;
        uint16_t mPostDelay = 50;
        uint32_t mBaudRate = 9600;
        SerialConfig mPortConfig;
        HardwareSerial* mPort = nullptr;
    private:
    #if defined(DEBUG)
        uint8_t mPortIndexForDebugging = 0;
    #endif
    private:
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        static constexpr uint8_t RX_PIN_NUMBER = 16;
        static constexpr uint8_t TX_PIN_NUMBER = 17;
    #elif defined(MODLINK_T2)
        static constexpr uint8_t RX2_PIN_NUMBER = 16;
        static constexpr uint8_t RX3_PIN_NUMBER = 16;
        static constexpr uint8_t TX2_PIN_NUMBER = 17;
        static constexpr uint8_t TX3_PIN_NUMBER = 17;
    #elif defined(MODLINK_B)
        static constexpr uint8_t RX2_PIN_NUMBER = 16;
        static constexpr uint8_t RX3_PIN_NUMBER = 16;
        static constexpr uint8_t TX2_PIN_NUMBER = 17;
        static constexpr uint8_t TX3_PIN_NUMBER = 17;
    #endif
    };
}