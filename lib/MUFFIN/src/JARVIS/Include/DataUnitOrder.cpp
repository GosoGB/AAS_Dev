/**
 * @file DataUnitOrder.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Data unit 정렬 순서를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "DataUnitOrder.h"



namespace muffin { namespace jvs {

    DataUnitOrder::DataUnitOrder(const uint8_t sizeOfOrder)
    {
        ASSERT((sizeOfOrder != 0), "THE SIZE OF ORDER CANNOT BE 0");
        mVectorOrder.reserve(sizeOfOrder);
    }
    
    DataUnitOrder::~DataUnitOrder()
    {
    }

    DataUnitOrder& DataUnitOrder::operator=(const DataUnitOrder& obj)
    {
        if (this != &obj)
        {
            mVectorOrder = obj.mVectorOrder;
        }
        
        return *this;
    }
    
    bool DataUnitOrder::operator==(const DataUnitOrder& obj) const
    {
        if (mVectorOrder.size() != obj.mVectorOrder.size())
        {
            return false;
        }
        
        for (size_t i = 0; i < mVectorOrder.size(); ++i)
        {
            if ((mVectorOrder[i].ByteOrder != obj.mVectorOrder[i].ByteOrder) ||
                (mVectorOrder[i].DataUnit  != obj.mVectorOrder[i].DataUnit)  ||
                (mVectorOrder[i].Index     != obj.mVectorOrder[i].Index))
            {
                return false;
            }
        }
        
        return true;
    }
    
    bool DataUnitOrder::operator!=(const DataUnitOrder& obj) const
    {
        return !(*this == obj);
    }
    
    std::vector<ord_t>::iterator DataUnitOrder::begin()
    {
        return mVectorOrder.begin();
    }

    std::vector<ord_t>::iterator DataUnitOrder::end()
    {
        return mVectorOrder.end();
    }

    std::vector<ord_t>::const_iterator DataUnitOrder::begin() const
    {
        return mVectorOrder.cbegin();
    }

    std::vector<ord_t>::const_iterator DataUnitOrder::end() const
    {
        return mVectorOrder.cend();
    }
    
    uint8_t DataUnitOrder::GetSize() const
    {
        ASSERT((mVectorOrder.size() <= UINT8_MAX), "THE NUMBER OF DATA UNIT ORDERS CANNOT EXCEED 255");

        return static_cast<uint8_t>(mVectorOrder.size());
    }

    uint8_t DataUnitOrder::GetCapacity() const
    {
        ASSERT((mVectorOrder.capacity() <= UINT8_MAX), "CAPACITY OF DATA UNIT ORDERS CANNOT EXCEED 255");

        return static_cast<uint8_t>(mVectorOrder.capacity());
    }

    Status DataUnitOrder::EmplaceBack(const ord_t order)
    {
        ASSERT((mVectorOrder.size() < UINT8_MAX), "ORDER CANNOT BE EMPLACED BEYOND ITS DESIGNED CAPACITY");

        std::exception exception;
        Status ret(Status::Code::UNCERTAIN);

        try
        {
            mVectorOrder.emplace_back(order);
            ret = Status::Code::GOOD;
            return ret;
        }
        catch(const std::bad_alloc& e)
        {
            exception = e;
            ret = Status::Code::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            exception = e;
            ret = Status::Code::BAD_UNEXPECTED_ERROR;
        }

        LOG_ERROR(logger, "%s", exception.what());
        return ret;
    }

    std::pair<Status, ord_t> DataUnitOrder::Retrieve(const uint8_t index) const
    {
        if ((index + 1) > mVectorOrder.size())
        {
            ord_t order;
            order.DataUnit   = data_unit_e::BYTE;
            order.ByteOrder  = byte_order_e::LOWER;
            order.Index      = 0;

            return std::make_pair(Status(Status::Code::BAD_OUT_OF_RANGE), order);
        }
        else
        {
            return std::make_pair(Status(Status::Code::GOOD), mVectorOrder[index]);
        }
    }
    
    size_t DataUnitOrder::RetrieveTotalSize() const
    {
        ASSERT((mVectorOrder.size() != 0), "TOTAL SIZE CANNOT BE CALCULATED WHEN THERE IS NO DATA UNIT ORDER EMPLACED");

        size_t sum = 0;

        for (const auto& order : mVectorOrder)
        {
            sum += static_cast<uint8_t>(order.DataUnit);
        }
        
        return sum;
    }
}}