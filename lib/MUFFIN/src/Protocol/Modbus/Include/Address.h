/**
 * @file Address.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus RTU 슬레이브에 대한 주소 테이블을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <set>

#include "IM/Node/Include/NumericAddressRange.h"
#include "TypeDefinitions.h"



namespace muffin { namespace modbus {

    class Address
    {
    public:
        Address();
        ~Address();
    public:
        void UpdateAddressMap(const area_e area, const muffin::im::NumericAddressRange& range);
        std::set<im::NumericAddressRange> GetAddress(const area_e area);
    private:
        void updateConsecutiveRanges(std::set<muffin::im::NumericAddressRange>* range);
    private:
        void printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const;
        void printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const;
        void printAddressMap() const;
    private:
        std::map<area_e, std::set<im::NumericAddressRange>> mAddressMap;
    };

    // area_e Address::GetArea() const
    // {
    //     return mArea;
    // }
}}