/**
 * @file bitset.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief bitset 자료구조를 표현하는 클래스를 정의합니다.
 * @details 일련의 항목 또는 조건에 대한 플래그를 유지하는 
 *          간단한 방법을 제공하는 고정 비트 수로 구성된 
 *          시퀀스를 저장하는 개체를 제공합니다.
 * 
 * @date 2025-01-14
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <sys/_stdint.h>
#include <type_traits>

#include "Common/Assert.h"

constexpr const char* ERROR_OUT_OF_RANGE = "INDEX OUT OF RANGE";



namespace muffin {

    template <uint8_t T>
    class bitset
    {
        static_assert(T > 0, "CAPACITY MUST BE GREATER THAN 0");
        static_assert(T < 65, "CAPACITY MUST BE SMALLER THAN 65");
        using DataType = typename std::conditional<(T <=  8), uint8_t,
                         typename std::conditional<(T <= 16), uint16_t,
                         typename std::conditional<(T <= 32), uint32_t,
                         uint64_t>::type>::type>::type;

    public:
        bitset() : mData(0) {}
        ~bitset() {}

    public:
        void set(const uint8_t index)
        {
            ASSERT((index < T), ERROR_OUT_OF_RANGE);
            mData |= (DataType(1) << index);
        }

        bool test(const uint8_t index) const
        {
            ASSERT((index < T), ERROR_OUT_OF_RANGE);
            return (mData & (DataType(1) << index)) >> index;
        }

        bool all()
        {
            return mData == ((DataType(1) << T) - 1);
        }

        bool any()
        {
            return mData != 0;
        }
        
        bool none()
        {
            return mData == 0;
        }

        void flip()
        {
            mData = ~mData;
        }

        void flip(const uint8_t index)
        {
            ASSERT((index < T), ERROR_OUT_OF_RANGE);
            mData ^= ~(DataType(1) << index);
        }

        void reset()
        {
            mData = 0;
        }

        void reset(const uint8_t index)
        {
            ASSERT((index < T), ERROR_OUT_OF_RANGE);
            mData &= ~(DataType(1) << index);
        }

    private:
        DataType mData;
    };
}