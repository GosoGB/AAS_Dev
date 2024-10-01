/**
 * @file Address.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Address.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace modbus {

    Address::Address()
    {
        mAddressMap.emplace(area_e::COIL,             std::set<im::NumericAddressRange>());
        mAddressMap.emplace(area_e::DISCRETE_INPUT,   std::set<im::NumericAddressRange>());
        mAddressMap.emplace(area_e::INPUT_REGISTER,   std::set<im::NumericAddressRange>());
        mAddressMap.emplace(area_e::HOLDING_REGISTER, std::set<im::NumericAddressRange>());

    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Address::~Address()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void Address::UpdateAddressMap(const area_e area, const im::NumericAddressRange& range)
    {
        auto it = mAddressMap.find(area);
        auto& ranges = it->second;

        if (ranges.size() == 0)
        {
            ranges.emplace(range);
            LOG_VERBOSE(logger, "Added a new range to an empty set");
            return ;
        }
        
        for (auto it = ranges.begin(); it != ranges.end(); it++)
        {
            if (it->IsMergeable(range) == true)
            {
                im::NumericAddressRange retrievedRange = *it;
                retrievedRange.MergeRanges(range);

                ranges.erase(it);
                ranges.emplace(retrievedRange);
                it = ranges.begin();
                LOG_VERBOSE(logger, "Merged the given range to the previous range set");
                return ;
            }
        }
        
        ranges.erase(range);
        LOG_VERBOSE(logger, "Added a new range to the previous range set");
    }

    std::set<area_e> Address::RetrieveAreaSet() const
    {
        std::set<area_e> areaSet;

        for (const auto& address : mAddressMap)
        {
            if (address.second.size() != 0)
            {
                areaSet.emplace(address.first);
            }
        }

        return areaSet;
    }

    const std::set<im::NumericAddressRange>& Address::RetrieveAddressSet(const area_e area) const
    {
        auto it = mAddressMap.find(area);
        return it->second;
    }

    void Address::updateConsecutiveRanges(std::set<muffin::im::NumericAddressRange>* range)
    {
        ASSERT((range != nullptr), "RANGE MUST NOT BE A NULL POINTER");
        LOG_VERBOSE(logger, "Start to update consecutive ranges to merge");

        for (auto it = range->begin(); it != std::prev(range->end()); ++it)
        {
            if (it->IsMergeable(*std::next(it)) == true)
            {
                im::NumericAddressRange formerRange = *it;
                im::NumericAddressRange latterRange = *std::next(it);
                range->erase(std::next(it));
                it = range->erase(it);
                formerRange.MergeRanges(latterRange);
                range->emplace(formerRange);
                LOG_VERBOSE(logger, "Merged consecutive ranges");
                updateConsecutiveRanges(range);
            }
        }
    }
}}