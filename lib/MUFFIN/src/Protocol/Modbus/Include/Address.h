/**
 * @file Address.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-01
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
        std::set<area_e> RetrieveAreaSet() const;
        const std::set<im::NumericAddressRange>& RetrieveAddressSet(const area_e area) const;
    private:
        void updateConsecutiveRanges(std::set<muffin::im::NumericAddressRange>* range);
    private:
        std::map<area_e, std::set<im::NumericAddressRange>> mAddressMap;
    };
}}