/**
 * @file Address.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @todo Update 함수에서 연속된 주소 범위의 길이를 제한하는 기능을 추가해야 합니다.
 *       예를 들어, Modbus는 한 번의 폴링에서 최대 2,000 개의 연속된 주소를 읽을
 *       수 있으므로 주소 범위의 길이를 2,000개 이하로 제한하는 것입니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <set>

#include "IM/Node/Include/NumericAddressRange.h"
#include "Jarvis/Include/TypeDefinitions.h"
#include "TypeDefinitions.h"



namespace muffin { namespace modbus {

    class Address
    {
    public:
        Address();
        Address(const Address& obj);
        virtual ~Address();
    private:
        using AddressRange = im::NumericAddressRange;
        using AddressRangeSet = std::set<AddressRange>;
    public:
        Status Update(const jarvis::mb_area_e area, const AddressRange& range);
        Status Remove(const jarvis::mb_area_e area, const AddressRange& range);
    private:
        Status emplaceAddressRange(const jarvis::mb_area_e area, const AddressRange& range, AddressRangeSet* ranges);
        Status updateConsecutiveRanges(const jarvis::mb_area_e area, AddressRangeSet* ranges);
    public:
        std::pair<Status, std::set<jarvis::mb_area_e>> RetrieveArea() const;
        const std::set<AddressRange>& RetrieveAddressRange(const jarvis::mb_area_e area) const;
    private:
        std::map<jarvis::mb_area_e, std::set<AddressRange>> mMapAddressByArea;
    };
}}