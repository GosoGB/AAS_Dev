/**
 * @file PolledData.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-02
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Status.h"
#include "PolledData.h"
#include "TypeDefinitions.h"



namespace muffin { namespace modbus {

    PolledData::PolledData()
    {
    }
    
    PolledData::~PolledData()
    {
    }

    Status PolledData::UpdateCoil(const uint16_t address, const bool value)
    {
        if (mMapPolledDataByArea.find(area_e::COIL) == mMapPolledDataByArea.end())
        {
            mMapPolledDataByArea.emplace(std::make_pair(area_e::COIL, std::vector<datum_t>()));
        }
        
        const uint16_t vectorIndex = static_cast<uint16_t>(address / 16.0f);
        const uint8_t bitIndex = (address - (vectorIndex * 16)) % 16;

        datum_t datum;
        datum.Address = vectorIndex;

        if (value == true)
        {
            datum.Value |= 1 << bitIndex;
        }
        else if (value == false)
        {
            datum.Value &= ~(1 << bitIndex);
        }
        else
        {
            return Status(Status::Code::BAD_DATA_UNAVAILABLE);
        }

        const auto itBegin = mMapPolledDataByArea[area_e::COIL].begin();
        const auto itEnd   = mMapPolledDataByArea[area_e::COIL].end();
        auto itCurrent = itBegin;

        for (; itCurrent != itEnd; ++itCurrent)
        {
            if (itCurrent->Address == vectorIndex)
            {
                break;
            }
        }

        if (itCurrent == itEnd)
        {
            mMapPolledDataByArea[area_e::COIL].emplace_back(datum);
        }
        else
        {
            itCurrent->Value = datum.Value;
        }

        return Status(Status::Code::GOOD);
    }

    bool PolledData::RetrieveCoil(const uint16_t address) const
    {
        ASSERT((mMapPolledDataByArea.find(area_e::COIL) != mMapPolledDataByArea.end()), "NO DATA POLLED IN THE AREA");;
        
        const uint16_t vectorIndex = static_cast<uint16_t>(address / 16.0f);
        const uint8_t bitIndex = (address - (vectorIndex * 16)) % 16;

        const auto itBegin = mMapPolledDataByArea.at(area_e::COIL).begin();
        const auto itEnd   = mMapPolledDataByArea.at(area_e::COIL).end();
        auto itCurrent = itBegin;

        for (; itCurrent != itEnd; ++itCurrent)
        {
            if (itCurrent->Address == vectorIndex)
            {
                break;
            }
        }
        
        return itCurrent->Value >> bitIndex & 1;
    }
}}