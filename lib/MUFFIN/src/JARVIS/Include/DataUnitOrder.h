/**
 * @file DataUnitOrder.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Data unit 정렬 순서를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-09
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "TypeDefinitions.h"



namespace muffin { namespace jvs {

    class DataUnitOrder
    {
    public:
        explicit DataUnitOrder(const uint8_t sizeOfOrder);
        virtual ~DataUnitOrder();
    public:
        DataUnitOrder& operator=(const DataUnitOrder& obj);
        bool operator==(const DataUnitOrder& obj) const;
        bool operator!=(const DataUnitOrder& obj) const;
    public:
        std::vector<ord_t>::iterator begin();
        std::vector<ord_t>::iterator end();
        std::vector<ord_t>::const_iterator begin() const;
        std::vector<ord_t>::const_iterator end() const;
    public:
        uint8_t GetSize() const;
        uint8_t GetCapacity() const;
        Status EmplaceBack(const ord_t order);
        std::pair<Status, ord_t> Retrieve(const uint8_t index) const;
        size_t RetrieveTotalSize() const;
    private:
        std::vector<ord_t> mVectorOrder;
    };
}}