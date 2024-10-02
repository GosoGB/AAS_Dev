/**
 * @file NumericAddressRange.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 숫자형 주소의 범위를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <sys/_stdint.h>

#include "Common/Status.h"



namespace muffin { namespace im {

    class NumericAddressRange
    {
    public:
        NumericAddressRange(const uint16_t startAddress, const uint16_t quantity);
        ~NumericAddressRange();
    public:
        bool operator<(const NumericAddressRange& obj) const;
    public:
        bool IsMergeable(const NumericAddressRange& obj) const;
        void MergeRanges(const NumericAddressRange& obj);
    public:
        bool IsRemovable(const NumericAddressRange& obj) const;
        void Remove(const NumericAddressRange& obj, bool* isRemovableRange, uint16_t* remainedAddress, uint16_t* remainedQuantity);
    public:
        uint16_t GetStartAddress() const;
        uint16_t GetLastAddress() const;
        uint16_t GetQuantity() const;
    private:
        uint16_t mStart;
        uint16_t mQuantity;
        static constexpr uint8_t CONSECUTIVE_DIFFERENCE = 1;
    };
}}