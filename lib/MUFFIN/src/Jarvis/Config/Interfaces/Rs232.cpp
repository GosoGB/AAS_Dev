/**
 * @file Rs232.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief RS-232 시리얼 포트 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @todo ModbusRTU 설정 형식을 참조하여 클래스를 전반적으로 수정해야 합니다. (@김주성)
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Logger/Logger.h"
#include "Rs232.h"


namespace muffin { namespace jarvis { namespace config {

    Rs232::Rs232(const std::string& key)
        : Base(key)
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Rs232::~Rs232()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Rs232& Rs232::operator=(const Rs232& obj)
    {
        if (this != &obj)
        {
            mPortName  = obj.mPortName;
            mParityBit = obj.mParityBit;
            mBaudRate  = obj.mBaudRate;
            mDataBit   = obj.mDataBit;
            mStopBit   = obj.mStopBit;
        }
        
        return *this;
    }

    bool Rs232::operator==(const Rs232& obj) const
    {
       return (
            mPortName  == obj.mPortName    &&
            mParityBit == obj.mParityBit   &&
            mBaudRate  == obj.mBaudRate    &&
            mDataBit   == obj.mDataBit     &&
            mStopBit   == obj.mStopBit     
        );
    }

    bool Rs232::operator!=(const Rs232& obj) const
    {
        return !(*this == obj);
    }

    Status Rs232::SetPortName(const uint8_t& name)
    {
        mPortName = name;
        if (mPortName == name)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Rs232::SetBaudRate(const uint32_t& baudrate)
    {
        mBaudRate = baudrate;
        if (mBaudRate == baudrate)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Rs232::SetDataBit(const uint8_t& databit)
    {
        mDataBit = databit;
        if (mDataBit == databit)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Rs232::SetParityBit(const uint8_t& paritybit)
    {
        mParityBit = paritybit;
        if (mParityBit == paritybit)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Rs232::SetStopBit(const uint8_t& stopbit)
    {
        mStopBit = stopbit;
        if (mStopBit == stopbit)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    const uint8_t& Rs232::GetPortName() const
    {
        return mPortName;
    }

    const uint32_t& Rs232::GetBaudRate() const
    {
        return mBaudRate;
    }

    const uint8_t& Rs232::GetDataBit() const
    {
        return mDataBit;
    }

    const uint8_t& Rs232::GetParityBit() const
    {
        return mParityBit;
    }

    const uint8_t& Rs232::GetStopBit() const
    {
        return mStopBit;
    }

}}}


