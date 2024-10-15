/**
 * @file Rs232.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief RS-232 시리얼 포트 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
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
        Rs232();
        virtual ~Rs232() override;
    public:
        Rs232& operator=(const Rs232& obj);
        bool operator==(const Rs232& obj) const;
        bool operator!=(const Rs232& obj) const;
    public:
        void SetPortIndex(const prt_e index);
        void SetBaudRate(const bdr_e baudRate);
        void SetDataBit(const dbit_e dataBit);
        void SetParityBit(const pbit_e parityBit);
        void SetStopBit(const sbit_e stopBit);
    public:
        std::pair<Status, prt_e> GetPortIndex() const;
        std::pair<Status, bdr_e> GetBaudRate() const;
        std::pair<Status, dbit_e> GetDataBit() const;
        std::pair<Status, pbit_e> GetParityBit() const;
        std::pair<Status, sbit_e> GetStopBit() const;
    private:
        bool mIsPortIndexSet  = false;
        bool mIsBaudRateSet   = false;
        bool mIsDataBitSet    = false;
        bool mIsParityBitSet  = false;
        bool mIsStopBitSet    = false;
    private:
        prt_e mPortIndex;
        bdr_e mBaudRate;
        dbit_e mDataBit;
        pbit_e mParityBit;
        sbit_e mStopBit;
    };
}}}
