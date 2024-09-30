/**
 * @file Address.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus RTU 슬레이브에 대한 주소 테이블을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

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
        auto& rangeRetrieved = it->second;

        if (rangeRetrieved.size() == 0)
        {
            rangeRetrieved.emplace(range);
            LOG_VERBOSE(logger, "Added a new range to an empty set");
            return ;
        }

        for (auto it = rangeRetrieved.begin(); it != rangeRetrieved.end(); ++it)
        {
            if (it->IsMergeable(range) == true)
            {
                im::NumericAddressRange storedRange = *it;
                storedRange.MergeRanges(range);

                rangeRetrieved.erase(it);
                rangeRetrieved.emplace(storedRange);
                
                LOG_VERBOSE(logger, "Merged the given range to the previous range set");
                goto UPDATE_CONSECUTIVE_RAGES;
            }
        }

        LOG_VERBOSE(logger, "Added a new range to the previous range set");
        rangeRetrieved.emplace(range);

    UPDATE_CONSECUTIVE_RAGES:
        updateConsecutiveRanges(&rangeRetrieved);
        printAddressMap();
    }

    std::set<im::NumericAddressRange> Address::GetAddress(const area_e area)
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
    
    void Address::printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %-*s", cellWidth - 2, value);
        strcat(buffer, cell);
    }

    void Address::printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %*u", cellWidth - 2, value);
        strcat(buffer, cell);
    }

    void Address::printAddressMap() const
    {
        const char* dashLine = "----------------------------------------------\n";
        const uint16_t bufferSize = 512;
        const uint8_t cellWidth = 10;
        
        char buffer[bufferSize];
        uint8_t* castedBuffer = reinterpret_cast<uint8_t*>(buffer);
        memset(buffer, '\0', bufferSize * sizeof(char));
        strcat(buffer, dashLine);
        strcat(buffer, "| Area   | Index  | Start  | Last   | Qty.   |\n");
        strcat(buffer, dashLine);

        for (const auto& address : mAddressMap)
        {
            size_t idx = 1;
            const area_e& area = address.first;
            const std::set<muffin::im::NumericAddressRange>& ranges = address.second;

            const char* strArea = area == area_e::COIL           ? "COIL" : 
                                  area == area_e::DISCRETE_INPUT ? "D.I." :
                                  area == area_e::INPUT_REGISTER ? "I.R." : "H.R.";

            for (const auto& range : ranges)
            {
                printCell(cellWidth, strArea, castedBuffer);
                printCell(cellWidth, idx, castedBuffer);
                printCell(cellWidth, range.GetStartAddress(), castedBuffer);
                printCell(cellWidth, range.GetLastAddress(), castedBuffer);
                printCell(cellWidth, range.GetQuantity(), castedBuffer);
                strcat(buffer, "|\n");
                ++idx;
            }
        }

        strcat(buffer, dashLine);
        LOG_INFO(logger, "Modbus Address Table\n%s\n", buffer);
    }
}}