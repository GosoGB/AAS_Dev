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




#include <algorithm>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "PolledData.h"



namespace muffin { namespace modbus {

    PolledData::PolledData()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    PolledData::~PolledData()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status PolledData::UpdateCoil(const uint16_t address, const int8_t value)
    {
        auto itArea = mMapDatumByArea.find(area_e::COIL);
        if (itArea == mMapDatumByArea.end())
        {
            try
            {
                auto result = mMapDatumByArea.emplace(area_e::COIL, std::vector<datum_t>());
                itArea = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), address);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), address);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        auto itDatum = std::find_if(itArea->second.begin(), itArea->second.end(), [address](const datum_t& datum2find)
        {
            return datum2find.Address == address;
        });

        datum_t datum;
        datum.Address = address;

        switch (value)
        {
        case 0:
        case 1:
            datum.Value = value;
            datum.IsOK  = true;
            break;
        default:
            datum.Value = 0;
            datum.IsOK  = false;
            break;
        }

        if (itDatum == itArea->second.end())
        {
            try
            {
                mMapDatumByArea[area_e::COIL].emplace_back(datum);
                return Status(Status::Code::GOOD);
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), address);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), address);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }
        else
        {
            itDatum->Value = datum.Value;
            itDatum->IsOK  = datum.IsOK;
            return Status(Status::Code::GOOD);
        }
    }

    datum_t PolledData::RetrieveCoil(const uint16_t address) const
    {
        std::vector<datum_t>::const_iterator itDatum;

        auto itArea = mMapDatumByArea.find(area_e::COIL);
        if (itArea == mMapDatumByArea.end())
        {
            goto BAD_NO_DATA;
        }

        itDatum = std::find_if(itArea->second.begin(), itArea->second.end(), [address](const datum_t& datum2find)
        {
            return datum2find.Address == address;
        });

        if (itDatum != itArea->second.end())
        {
            return *itDatum;
        }

    BAD_NO_DATA:
        datum_t datum { .Address = address, .Value = 0, .IsOK = false };
        return datum;
    }
}}