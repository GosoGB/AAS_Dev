/**
 * @file Address.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Address.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace modbus {

    Address::Address()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Address::Address(const Address& obj)
        : mMapAddressByArea(obj.mMapAddressByArea)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed by Copy from %p to %p", &obj, this);
    #endif
    }

    Address::~Address()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status Address::Update(const jarvis::mb_area_e area, const AddressRange& range)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapAddressByArea.find(area);
        if (it == mMapAddressByArea.end())
        {
            try
            {
                auto result = mMapAddressByArea.emplace(area, std::set<AddressRange>());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u, [%u, %u]", e.what(), static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u, [%u, %u]", e.what(), static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
            LOG_VERBOSE(logger, "Emplaced a new area-range pair to the address map");
        }
        
        auto& addressRangeSet = it->second;
        if (addressRangeSet.size() == 0)
        {
            ret = emplaceAddressRange(area, range, &addressRangeSet);
            if (ret == Status::Code::GOOD)
            {
                LOG_VERBOSE(logger, "Emplaced the given address range to an empty set");
            }
            else
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE ADDRESS RANGE: %s", ret.c_str());
            }

            return ret;
        }
        
        for (auto itRange = addressRangeSet.begin(); itRange != addressRangeSet.end(); ++itRange)
        {
            if (itRange->IsMergeable(range) == true)
            {
                AddressRange retrieved = *itRange;
                addressRangeSet.erase(itRange);
                retrieved.MergeRanges(range);

                ret = emplaceAddressRange(area, retrieved, &addressRangeSet);
                if (ret == Status::Code::GOOD)
                {
                    LOG_VERBOSE(logger, "Merged the given address range");
                    goto UPDATE_CONSECUTIVES;
                }
                else
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE ADDRESS RANGE: %s", ret.c_str());
                    return ret;
                }
            }
        }
        
        ret = emplaceAddressRange(area, range, &addressRangeSet);
        if (ret == Status::Code::GOOD)
        {
            LOG_VERBOSE(logger, "Added a new address range");
            goto UPDATE_CONSECUTIVES;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE ADDRESS RANGE: %s", ret.c_str());
            return ret;
        }
    
    UPDATE_CONSECUTIVES:
        ret = updateConsecutiveRanges(area, &addressRangeSet);
        if (ret == Status::Code::GOOD)
        {
            LOG_VERBOSE(logger, "Updated the address map for area code: %u", static_cast<uint8_t>(area));
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO UPDATE THE ADDRESS MAP FOR AREA CODE: %u", static_cast<uint8_t>(area));
        }
        return ret;
    }

    Status Address::emplaceAddressRange(const jarvis::mb_area_e area, const AddressRange& range, AddressRangeSet* ranges)
    {
        std::exception exception;
        Status::Code retCode;

        try
        {
            auto result = ranges->emplace(range);
            ASSERT((result.second == true), "FAILED TO EMPLACE ADDRESS RANGE TO THE SET SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            exception = e;
            retCode = Status::Code::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            exception = e;
            retCode = Status::Code::BAD_UNEXPECTED_ERROR;
        }

        LOG_ERROR(logger, "%s: %u, [%u, %u]", exception.what(), static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
        return Status(retCode);
    }

    Status Address::updateConsecutiveRanges(const jarvis::mb_area_e area, AddressRangeSet* ranges)
    {
        ASSERT((ranges != nullptr), "ADDRESS RANGE SET CANNOT BE A NULL POINTER");
        
        LOG_DEBUG(logger, "Start to update consecutive ranges to merge");
        Status ret(Status::Code::GOOD);

        bool hasMergeableRange = true;
        while (hasMergeableRange == true)
        {
            hasMergeableRange = false;

            for (auto it = ranges->begin(); std::next(it) != ranges->end(); ++it)
            {
                if (it->IsMergeable(*std::next(it)) == true)
                {
                    ASSERT((std::next(it) != ranges->end()), "std::next(it) cannot be end of ranges");
                    ASSERT((it != ranges->end()), "it cannot be end of ranges");

                    AddressRange formerRange = *it;
                    AddressRange latterRange = *std::next(it);
                    formerRange.MergeRanges(latterRange);
                    
                    ASSERT((std::next(it) != ranges->end()), "std::next(it) cannot be end of ranges");
                    ranges->erase(std::next(it));
                    ASSERT((it != ranges->end()), "it cannot be end of ranges");
                    ranges->erase(it);
                    it = ranges->begin();
                    LOG_DEBUG(logger, "it->GetLastAddress(): %u", it->GetLastAddress());
                    LOG_DEBUG(logger, "it->GetQuantity(): %u", it->GetQuantity());

                    ret = emplaceAddressRange(area, formerRange, ranges);
                    if (ret != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger, "FAILED TO UPDATE CONSECUTIVES: %s", ret.c_str());
                        return ret;
                    }
                    
                    LOG_VERBOSE(logger, "Merged consecutive ranges");
                    LOG_DEBUG(logger, "ranges size: %u", ranges->size());

                    hasMergeableRange = true;
                    break;
                }
            }
        }
        
        // for (auto it = ranges->begin(); std::next(it) != ranges->end(); ++it)
        // {
        //     LOG_DEBUG(logger, "ranges size: %u", ranges->size());
        //     if (it == ranges->end())
        //     {
        //         it = ranges->begin();
        //         continue;
        //     }
            
        //     if (it->IsMergeable(*std::next(it)) == true)
        //     {
        //         ASSERT((std::next(it) != ranges->end()), "std::next(it) cannot be end of ranges");
        //         ASSERT((it != ranges->end()), "it cannot be end of ranges");

        //         AddressRange formerRange = *it;
        //         AddressRange latterRange = *std::next(it);
        //         formerRange.MergeRanges(latterRange);
                
        //         ASSERT((std::next(it) != ranges->end()), "std::next(it) cannot be end of ranges");
        //         ranges->erase(std::next(it));
        //         ASSERT((it != ranges->end()), "it cannot be end of ranges");
        //         ranges->erase(it);
        //         it = ranges->begin();
        //         LOG_DEBUG(logger, "it->GetLastAddress(): %u", it->GetLastAddress());
        //         LOG_DEBUG(logger, "it->GetQuantity(): %u", it->GetQuantity());

        //         ret = emplaceAddressRange(area, formerRange, ranges);
        //         if (ret != Status::Code::GOOD)
        //         {
        //             LOG_ERROR(logger, "FAILED TO UPDATE CONSECUTIVES: %s", ret.c_str());
        //             return ret;
        //         }
                
        //         LOG_VERBOSE(logger, "Merged consecutive ranges");
        //         LOG_DEBUG(logger, "ranges size: %u", ranges->size());
        //     }
        // }
        LOG_DEBUG(logger, "End of updating consecutive ranges to merge");
        return ret;
    }

    Status Address::Remove(const jarvis::mb_area_e area, const AddressRange& range)
    {
        auto& addressRangeSet = mMapAddressByArea.find(area)->second;
        if (addressRangeSet.size() == 0)
        {
            goto NO_DATA_TO_REMOVE;
        }
        
        for (auto it = addressRangeSet.begin(); it != addressRangeSet.end(); ++it)
        {
            if (it->IsRemovable(range) == false)
            {
                continue;
            }
            
            AddressRange retrieved = *it;
            addressRangeSet.erase(it);

            bool isRemovableRange;
            uint16_t remainedAddress  = 0;
            uint16_t remainedQuantity = 0;

            Status ret = retrieved.Remove(range, &isRemovableRange, &remainedAddress, &remainedQuantity);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO REMOVE: %s", ret.c_str());
                return ret;
            }

            if (isRemovableRange == true)
            {
                LOG_VERBOSE(logger, "Removed the entire address range");
                return Status(Status::Code::GOOD);
            }
            else if (remainedAddress != 0 && remainedQuantity != 0)
            {
                AddressRange remainedRange(remainedAddress, remainedQuantity);
                for (const auto& e : addressRangeSet)
                {
                    LOG_DEBUG(logger, "%u", e.GetStartAddress());
                }

                Status ret = emplaceAddressRange(area, retrieved, &addressRangeSet);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE THE HEAD OF THE REMAINED ADDRESS RANGE");
                    return ret;
                }

                ret = emplaceAddressRange(area, remainedRange, &addressRangeSet);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE THE TAIL OF THE REMAINED ADDRESS RANGE");
                    return ret;
                }
                
                LOG_VERBOSE(logger, "Removed middle of a range set and remained parts were emplaced");
                return ret;
            }
            else
            {
                Status ret = emplaceAddressRange(area, retrieved, &addressRangeSet);
                if (ret == Status::Code::GOOD)
                {
                    LOG_VERBOSE(logger, "Removed head or tail part of given address range");
                }
                else
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE THE HEAD OR TAIL PART OF GIVEN ADDRESS RANGE");
                }
                return ret;
            }
        }

    NO_DATA_TO_REMOVE:
        LOG_VERBOSE(logger, "No address in the range to remove");
        return Status(Status::Code::GOOD_NO_DATA);
    }

    std::pair<Status, std::set<jarvis::mb_area_e>> Address::RetrieveArea() const
    {
        std::exception exception;
        Status::Code retCode;

        try
        {
            std::set<jarvis::mb_area_e> areas;
            for (const auto& pair : mMapAddressByArea)
            {
                areas.emplace(pair.first);
            }
            return std::make_pair(Status(Status::Code::GOOD), areas);
        }
        catch(const std::bad_alloc& e)
        {
            exception = e;
            retCode = Status::Code::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            exception = e;
            retCode = Status::Code::BAD_UNEXPECTED_ERROR;
        }
        
        LOG_ERROR(logger, "%s", exception.what());
        return std::make_pair(Status(retCode), std::set<jarvis::mb_area_e>());
    }

    const std::set<im::NumericAddressRange>& Address::RetrieveAddressRange(const jarvis::mb_area_e area) const
    {
        return mMapAddressByArea.find(area)->second;
    }
}}