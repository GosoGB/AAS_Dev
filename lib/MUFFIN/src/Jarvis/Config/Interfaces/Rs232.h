/**
 * @file Rs232.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief RS-232 시리얼 포트 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @todo ModbusRTU 설정 형식을 참조하여 클래스를 전반적으로 수정해야 합니다. (@김주성)
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {


    class Rs232 : public Base
    {
    public:
        Rs232(const std::string& key);
        virtual ~Rs232() override;
    public:
        Rs232& operator=(const Rs232& obj);
        bool operator==(const Rs232& obj) const;
        bool operator!=(const Rs232& obj) const;
    
    public:
        Status SetPortName(const uint8_t& name);
        Status SetBaudRate(const uint32_t& baudrate);
        Status SetDataBit(const uint8_t& databit);
        Status SetParityBit(const uint8_t& paritybit);
        Status SetStopBit(const uint8_t& stopbit);
    
    public:
        const uint8_t& GetPortName() const;
        const uint32_t& GetBaudRate() const;
        const uint8_t& GetDataBit() const;
        const uint8_t& GetParityBit() const;
        const uint8_t& GetStopBit() const;

    private:
        bool mIsPortNameSet  = false;
        bool mIsBaudrateSet  = false;
        bool mIsDataBitSet   = false;
        bool mIsParityBitSet = false;
        bool mIsStopBitSet   = false;

    private:
        uint8_t mPortName;
        uint8_t mParityBit;
        uint32_t mBaudRate;
        uint8_t mDataBit;
        uint8_t mStopBit;
    };
}}}
