/**
 * @file Rs485.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief RS-485 시리얼 포트 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Rs485.h"



namespace muffin { namespace jarvis { namespace config {

    Rs485::Rs485()
        : Base(cfg_key_e::RS485)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Rs485::~Rs485()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Rs485& Rs485::operator=(const Rs485& obj)
    {
        if (this != &obj)
        {
            mPortIndex  = obj.mPortIndex;
            mParityBit  = obj.mParityBit;
            mBaudRate   = obj.mBaudRate;
            mDataBit    = obj.mDataBit;
            mStopBit    = obj.mStopBit;
        }
        
        return *this;
    }

    bool Rs485::operator==(const Rs485& obj) const
    {
       return (
            mPortIndex  == obj.mPortIndex   &&
            mParityBit  == obj.mParityBit   &&
            mBaudRate   == obj.mBaudRate    &&
            mDataBit    == obj.mDataBit     &&
            mStopBit    == obj.mStopBit     
        );
    }

    bool Rs485::operator!=(const Rs485& obj) const
    {
        return !(*this == obj);
    }

    void Rs485::SetPortIndex(const prt_e index)
    {
        mPortIndex = index;
        mIsPortIndexSet = true;
    }

    void Rs485::SetBaudRate(const bdr_e baudRate)
    {
        mBaudRate = baudRate;
        mIsBaudRateSet = true;
    }

    void Rs485::SetDataBit(const dbit_e dataBit)
    {
        mDataBit = dataBit;
        mIsDataBitSet = true;
    }

    void Rs485::SetParityBit(const pbit_e parityBit)
    {
        mParityBit = parityBit;
        mIsParityBitSet = true;
    }

    void Rs485::SetStopBit(const sbit_e stopBit)
    {
        mStopBit = stopBit;
        mIsStopBitSet = true;
    }

    std::pair<Status, prt_e> Rs485::GetPortIndex() const
    {
        if (mIsPortIndexSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mPortIndex);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPortIndex);
        }
    }

    std::pair<Status, bdr_e> Rs485::GetBaudRate() const
    {
        if (mIsBaudRateSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mBaudRate);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mBaudRate);
        }
    }

    std::pair<Status, dbit_e> Rs485::GetDataBit() const
    {
        if (mIsDataBitSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDataBit);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDataBit);
        }
    }

    std::pair<Status, pbit_e> Rs485::GetParityBit() const
    {
        if (mIsParityBitSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mParityBit);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mParityBit);
        }
    }

    std::pair<Status, sbit_e> Rs485::GetStopBit() const
    {
        if (mIsStopBitSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mStopBit);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mStopBit);
        }
    }
}}}
