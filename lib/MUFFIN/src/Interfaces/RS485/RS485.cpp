/**
 * @file RS485.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief RS-485/422 시리얼 통신 인터페이스 클래스를 정의합니다.
 * 
 * @date 2024-10-21
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "RS485.h"



namespace muffin {

    /**
     * @brief 실제 하드웨어에 따라 생성할 수 있는 개체의 수가 제한되도록 해야 합니다.
     */
    RS485* RS485::GetInstanceOrNull()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) RS485();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS-485 INTERFACE");
                return mInstance;
            }
        }

        return mInstance;
    }

    RS485& RS485::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    RS485::operator bool()
    {
        return true;
    }

    RS485::RS485()
    {
    }
    
    RS485::~RS485()
    {
    }
    
    void RS485::Config(jarvis::config::Rs485* config)
    {
        const auto cfg = config->GetPortIndex();
        ASSERT((cfg.first.ToCode() == Status::Code::GOOD), "INVALID CONFIGURATION STATUS: %s", cfg.first.c_str());

        if (cfg.second == jarvis::prt_e::PORT_2)
        {
            mPort = &Serial2;
        }
    }

    void RS485::begin()
    {
        ASSERT((mIsConfigured == true), "CONFIGURATION MUST BE DONE BEFORE BEGINNING THE PORT");
        begin(mBaudRate, mPortConfig, mPreDelay, mPostDelay);
    }

    void RS485::begin(const uint32_t baudrate)
    {
        ASSERT((mIsConfigured == true), "CONFIGURATION MUST BE DONE BEFORE BEGINNING THE PORT");

        mBaudRate = baudrate;
        begin(mBaudRate, mPortConfig, mPreDelay, mPostDelay);
    }

    void RS485::begin(const uint32_t baudrate, const uint32_t config)
    {
        ASSERT((mIsConfigured == true), "CONFIGURATION MUST BE DONE BEFORE BEGINNING THE PORT");
        
        mBaudRate = baudrate;
        mPortConfig = static_cast<SerialConfig>(config);
        begin(mBaudRate, mPortConfig, mPreDelay, mPostDelay);
    }

    void RS485::begin(const uint32_t baudrate, const uint16_t predelay, const uint16_t postdelay)
    {
        ASSERT((mIsConfigured == true), "CONFIGURATION MUST BE DONE BEFORE BEGINNING THE PORT");
        
        mBaudRate = baudrate;
        mPreDelay = predelay;
        mPostDelay = postdelay;
        begin(mBaudRate, mPortConfig, mPreDelay, mPostDelay);
    }

    void RS485::begin(const uint32_t baudrate, const uint32_t config, const uint16_t predelay, const uint16_t postdelay)
    {
        // constexpr uint8_t DEFAULT_DELAY_IN_MILLIS = 50;

        // mBaudRate      = baudrate;
        // mPortConfig    = static_cast<SerialConfig>(config);
        // mPreDelay      = predelay < DEFAULT_DELAY_IN_MILLIS  ? DEFAULT_DELAY_IN_MILLIS : predelay;
        // mPostDelay     = postdelay < DEFAULT_DELAY_IN_MILLIS ? DEFAULT_DELAY_IN_MILLIS : predelay;

        // mPort->begin(mBaudRate, mPortConfig, RX_PIN_NUMBER, TX_PIN_NUMBER);
    }

    void RS485::end()
    {
        mPort->end();
    }

    int32_t RS485::available()
    {
        return mPort->available();
    }

    int32_t RS485::peek()
    {
        return mPort->peek();
    }

    int32_t RS485::read(void)
    {
        return mPort->read();
    }

    size_t RS485::write(const uint8_t byte)
    {
        if (mIsTransmissionBegun == false)
        {
            setWriteError();
            return 0;
        }

        return mPort->write(byte);
    }

    size_t RS485::write(const uint8_t* buffer, size_t size)
    {
        return mPort->write(buffer, size);
    }

    size_t RS485::write(const char* buffer, size_t size)
    {
        const uint8_t* reinterpretedBuffer = reinterpret_cast<const uint8_t*>(buffer);
        return mPort->write(reinterpretedBuffer, size);
    }

    void RS485::beginTransmission()
    {
        delayMicroseconds(mPreDelay);
        mIsTransmissionBegun = true;
    }

    void RS485::endTransmission()
    {
        mPort->flush();
        delayMicroseconds(mPostDelay);
        mIsTransmissionBegun = false;
    }

    void RS485::receive()
    {
        /**
         * @note 현재 MODLINK 제품군은 RE 핀을 제어할 수 없습니다.
         *       다만 ArduinoModbus 라이브러리에서 호출하기 때문에
         *       의도적으로 비워둔 함수를 작성하게 되었습니다.
         * 
         * @todo 내부 Modbus 라이브러리를 만든 후 제거해야 합니다.
         */
    }

    void RS485::noReceive()
    {
        /**
         * @note 현재 MODLINK 제품군은 RE 핀을 제어할 수 없습니다.
         *       다만 ArduinoModbus 라이브러리에서 호출하기 때문에
         *       의도적으로 비워둔 함수를 작성하게 되었습니다.
         * 
         * @todo 내부 Modbus 라이브러리를 만든 후 제거해야 합니다.
         */
    }

    void RS485::sendBreak(const uint32_t duration)
    {
        // mPort->flush();
        // mPort->end();

        // pinMode(TX_PIN_NUMBER, OUTPUT);
        // digitalWrite(TX_PIN_NUMBER, LOW);

        // delay(duration);
        
        // mPort->begin(mBaudRate, mPortConfig, RX_PIN_NUMBER, TX_PIN_NUMBER);
    }

    void RS485::sendBreakMicroseconds(const uint32_t duration)
    {
        // mPort->flush();
        // mPort->end();

        // pinMode(TX_PIN_NUMBER, OUTPUT);
        // digitalWrite(TX_PIN_NUMBER, LOW);

        // delayMicroseconds(duration);

        // mPort->begin(mBaudRate, mPortConfig, RX_PIN_NUMBER, TX_PIN_NUMBER);
    }


    RS485* RS485::mInstance = nullptr;
}