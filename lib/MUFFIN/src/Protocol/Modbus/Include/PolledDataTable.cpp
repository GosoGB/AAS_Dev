/**
 * @file PolledDataTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-22
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "PolledDataTable.h"



namespace muffin { namespace modbus {

    PolledDataTable::PolledDataTable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    PolledDataTable::~PolledDataTable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status PolledDataTable::UpdateCoil(const uint8_t slaveID, const uint16_t address, const int8_t value)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            try
            {
                auto result = mMapPolledDataBySlave.emplace(slaveID, PolledData());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        return it->second.UpdateCoil(address, value);
    }

    Status PolledDataTable::UpdateDiscreteInput(const uint8_t slaveID, const uint16_t address, const int8_t value)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            try
            {
                auto result = mMapPolledDataBySlave.emplace(slaveID, PolledData());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        return it->second.UpdateDiscreteInput(address, value);
    }

    Status PolledDataTable::UpdateInputRegister(const uint8_t slaveID, const uint16_t address, const int32_t value)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            try
            {
                auto result = mMapPolledDataBySlave.emplace(slaveID, PolledData());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        return it->second.UpdateInputRegister(address, value);
    }

    Status PolledDataTable::UpdateHoldingRegister(const uint8_t slaveID, const uint16_t address, const int32_t value)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            try
            {
                auto result = mMapPolledDataBySlave.emplace(slaveID, PolledData());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u", e.what(), slaveID);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        return it->second.UpdateHoldingRegister(address, value);
    }

    datum_t PolledDataTable::RetrieveCoil(const uint8_t slaveID, const uint16_t address) const
    {
        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            datum_t datum { .Address = address, .Value = false, .IsOK = false };
            return datum;
        }
        
        return it->second.RetrieveCoil(address);
    }

    datum_t PolledDataTable::RetrieveDiscreteInput(const uint8_t slaveID, const uint16_t address) const
    {
        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            datum_t datum { .Address = address, .Value = false, .IsOK = false };
            return datum;
        }
        
        return it->second.RetrieveDiscreteInput(address);
    }

    datum_t PolledDataTable::RetrieveInputRegister(const uint8_t slaveID, const uint16_t address) const
    {
        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            datum_t datum { .Address = address, .Value = false, .IsOK = false };
            return datum;
        }
        
        return it->second.RetrieveInputRegister(address);
    }

    datum_t PolledDataTable::RetrieveHoldingRegister(const uint8_t slaveID, const uint16_t address) const
    {
        auto it = mMapPolledDataBySlave.find(slaveID);
        if (it == mMapPolledDataBySlave.end())
        {
            datum_t datum { .Address = address, .Value = false, .IsOK = false };
            return datum;
        }
        
        return it->second.RetrieveHoldingRegister(address);
    }

    void PolledDataTable::Clear()
    {
        mMapPolledDataBySlave.clear();
    }
}}