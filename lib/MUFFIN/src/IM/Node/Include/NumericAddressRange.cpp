/**
 * @file NumericAddressRange.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 숫자형 주소의 범위를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-28
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "NumericAddressRange.h"



namespace muffin { namespace im {
    
    NumericAddressRange::NumericAddressRange(const uint16_t startAddress, const uint16_t quantity)
        : mStart(startAddress)
        , mQuantity(quantity)
    {
        ASSERT((quantity != 0 ), "quantity cannot be 0");
    }
    
    NumericAddressRange::~NumericAddressRange()
    {
    }
    
    bool NumericAddressRange::operator<(const NumericAddressRange& obj) const
    {
        return mStart < obj.mStart;
    }

    /**
     * @todo 메모리 주소의 크기가 1만큼 차이나는 경우만 mergeable 가능하다고 판단합니다.
     *       이로 인해 주소의 크기가 2 이상 차이나는 경우에는 mergeable 하지 않다고 판단합니다.
     *       향후에는 이러한 경우에 어떻게 처리할 것인지 고민을 해봐야 합니다.
     */
    bool NumericAddressRange::IsMergeable(const NumericAddressRange& obj) const
    {        
        const uint16_t lastAddressInRange = GetLastAddress();
        const uint16_t lastAddressInGivenRange = obj.GetLastAddress();

        if ((lastAddressInRange >= obj.mStart) && (lastAddressInGivenRange >= mStart))
        {
            return true;
        }
        else if ((mStart - lastAddressInGivenRange == CONSECUTIVE_DIFFERENCE) ||
                 (obj.mStart - lastAddressInRange  == CONSECUTIVE_DIFFERENCE))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void NumericAddressRange::MergeRanges(const NumericAddressRange& obj)
    {
        ASSERT((IsMergeable(obj) == true), "CANNOT MERGE NON-OVERLAPPING RANGES");

        const uint16_t lastAddressInRange = GetLastAddress();
        const uint16_t lastAddressInGivenRange = obj.GetLastAddress();

        mStart = std::min(mStart, obj.mStart);
        mQuantity = std::max(lastAddressInRange, lastAddressInGivenRange) - mStart + 1;
    }

    bool NumericAddressRange::IsRemovable(const NumericAddressRange& obj) const
    {        
        const uint16_t lastAddressInGivenRange  = obj.GetLastAddress();
        const uint16_t lastAddressInRange = GetLastAddress();

        if ((lastAddressInGivenRange < mStart) || (lastAddressInRange < obj.mStart))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    Status NumericAddressRange::Remove(const NumericAddressRange& obj, bool* isRemovableRange, uint16_t* remainedAddress, uint16_t* remainedQuantity)
    {
        ASSERT((isRemovableRange != nullptr), "THE PARAMETER \"isRemovableRange\" CANNOT BE A NULL POINTER");
        ASSERT((remainedAddress  != nullptr), "THE PARAMETER \"remainedAddress\" CANNOT BE A NULL POINTER");
        ASSERT((remainedQuantity != nullptr), "THE PARAMETER \"remainedQuantity\" CANNOT BE A NULL POINTER");
        ASSERT((IsRemovable(obj) == true),    "CALL \"IsRemovable\" FUNCTION TO CHECK IF REMOVABLE PRIOR TO CALLING THIS FUNCTION");
        
        const uint16_t lastAddressInGivenRange  = obj.GetLastAddress();
        const uint16_t lastAddressInRange = GetLastAddress();

        if ((obj.mStart < mStart) || (lastAddressInRange < lastAddressInGivenRange))
        {
            return Status(Status::Code::BAD_OUT_OF_RANGE);
        }

        *isRemovableRange = false;
        *remainedAddress = 0;
        *remainedQuantity = 0;

        if (mStart == obj.mStart && lastAddressInRange == lastAddressInGivenRange)
        {
            *isRemovableRange = true;
        }
        else if (mStart == obj.mStart && lastAddressInRange != lastAddressInGivenRange)
        {
            mStart     = lastAddressInGivenRange + 1;
            mQuantity -= obj.mQuantity;
        }
        else if (mStart < obj.mStart && lastAddressInRange == lastAddressInGivenRange)
        {
            mQuantity = obj.mStart - mStart;
        }
        else if (mStart < obj.mStart && lastAddressInRange != lastAddressInGivenRange)
        {
            mQuantity = obj.mStart - mStart;
            *remainedAddress = obj.mStart + obj.mQuantity;
            *remainedQuantity = lastAddressInRange - lastAddressInGivenRange;
        }
        else
        {
            LOG_ERROR(logger, "UNDEFINED CONDITION FOR REMOVE OPERATION");
            ASSERT(false, "UNDEFINED CONDITION FOR REMOVE OPERATION");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        return Status(Status::Code::GOOD);
    }

    uint16_t NumericAddressRange::GetStartAddress() const
    {
        return mStart;
    }

    uint16_t NumericAddressRange::GetLastAddress() const
    {
        return mStart + mQuantity - 1;
    }

    uint16_t NumericAddressRange::GetQuantity() const
    {
        return mQuantity;
    }
}}