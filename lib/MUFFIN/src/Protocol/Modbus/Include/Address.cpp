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
        mMapAddressByArea.emplace(area_e::COIL,             std::set<im::NumericAddressRange>());
        mMapAddressByArea.emplace(area_e::DISCRETE_INPUT,   std::set<im::NumericAddressRange>());
        mMapAddressByArea.emplace(area_e::INPUT_REGISTER,   std::set<im::NumericAddressRange>());
        mMapAddressByArea.emplace(area_e::HOLDING_REGISTER, std::set<im::NumericAddressRange>());

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

    void Address::Update(const area_e area, const im::NumericAddressRange& range)
    {
        auto it = mMapAddressByArea.find(area);
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

    void Address::Remove(const area_e area, const im::NumericAddressRange& range)
    {
        auto& ranges = mMapAddressByArea.find(area)->second;

        if (ranges.size() == 0)
        {
            LOG_VERBOSE(logger, "No range in the set to remove");
            return ;
        }
        
        for (auto it = ranges.begin(); it != ranges.end(); ++it)
        {
            if (it->IsRemovable(range) == false)
            {
                continue;
            }
            
            im::NumericAddressRange retrievedRange = *it;
            ranges.erase(it);

            bool isRemovableRange;
            uint16_t remainedAddress  = 0;
            uint16_t remainedQuantity = 0;

            retrievedRange.Remove(range, &isRemovableRange, &remainedAddress, &remainedQuantity);
            if (isRemovableRange == true)
            {
                LOG_VERBOSE(logger, "Removed a range set");
                return ;
            }
            else if (remainedAddress != 0 && remainedQuantity != 0)
            {
                im::NumericAddressRange remainedRange(remainedAddress, remainedQuantity);
                for (const auto& e : ranges)
                {
                    LOG_DEBUG(logger, "%u", e.GetStartAddress());
                }
                
                const auto retHead = ranges.emplace(retrievedRange);
                const auto retTail = ranges.emplace(remainedRange);            
                ASSERT((retHead.second == true), "FAILED TO REMOVE ADDRESS RANGE: HEAD");
                ASSERT((retTail.second == true), "FAILED TO REMOVE ADDRESS RANGE: TAIL");
                LOG_VERBOSE(logger, "Removed middle of a range set and remained part was emplaced again");
                return ;
            }

            ranges.emplace(retrievedRange);
            LOG_VERBOSE(logger, "Removed head or tail part of a range set");
        }
    }

    std::set<area_e> Address::RetrieveAreaSet() const
    {
        std::set<area_e> areaSet;

        for (const auto& address : mMapAddressByArea)
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
        auto it = mMapAddressByArea.find(area);
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