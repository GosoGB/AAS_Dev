/**
 * @file NumericAddressRange.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 숫자형 주소의 범위를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
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
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    NumericAddressRange::~NumericAddressRange()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
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
            LOG_VERBOSE(logger, "Mergeable ranges: Overlapping");
            return true;
        }
        else if ((mStart - lastAddressInGivenRange == CONSECUTIVE_DIFFERENCE) ||
                 (obj.mStart - lastAddressInRange  == CONSECUTIVE_DIFFERENCE))
        {
            LOG_VERBOSE(logger, "Mergeable ranges: Consecutive");
            return true;
        }
        else
        {
            LOG_VERBOSE(logger, "Not a mergeable ranges");
            return false;
        }
    }

    void NumericAddressRange::MergeRanges(const NumericAddressRange& obj)
    {
        ASSERT((IsMergeable(obj) == true), "CANNOT MERGE NON-OVERLAPPING RANGES");
        LOG_VERBOSE(logger, "Range: [%u, %u]", mStart, mQuantity);
        LOG_VERBOSE(logger, "Given: [%u, %u]", obj.mStart, obj.mQuantity);
        
        mStart = std::min(mStart, obj.mStart);

        const uint16_t lastAddressInRange = GetLastAddress();
        const uint16_t lastAddressInGivenRange = obj.GetLastAddress();
        mQuantity = std::max(lastAddressInRange, lastAddressInGivenRange) - mStart + 1;
        
        LOG_VERBOSE(logger, "Merged: [%u, %u]", mStart, mQuantity);
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