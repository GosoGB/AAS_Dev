/**
 * @file AddressTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "AddressTable.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace modbus {
    
    AddressTable::AddressTable()
    {
    }
    
    AddressTable::~AddressTable()
    {
    }

    Status AddressTable::Update(const uint8_t slaveID, const jarvis::mb_area_e area, const AddressRange& range)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapAddressBySlave.find(slaveID);
        if (it == mMapAddressBySlave.end())
        {
            try
            {
                auto result = mMapAddressBySlave.emplace(slaveID, Address());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s: %u, %u, [%u, %u]", e.what(), slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "%s: %u, %u, [%u, %u]", e.what(), slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }

            // LOG_VERBOSE(logger, "Emplaced a new slave-address pair to the address table");
        }

        ret = it->second.Update(area, range);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS RANGE: %s", ret.c_str());
            return ret;
        }
        // LOG_VERBOSE(logger, "Address range updated: %u, %u, [%u, %u]", slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
    #if defined(DEBUG)
        printAddressTable();
    #else
        // CSV 형태로 로그를 만들어서 서버로 전송할 수 있게끔 해줘야 함
    #endif
    
        // countBufferSize();
        return ret;
    }

    Status AddressTable::Remove(const uint8_t slaveID, const jarvis::mb_area_e area, const AddressRange& range)
    {
        auto it = mMapAddressBySlave.find(slaveID);
        if (it == mMapAddressBySlave.end())
        {
            // LOG_VERBOSE(logger, "No matching slave ID");
            return Status(Status::Code::BAD_NOT_FOUND);
        }

        Status ret = it->second.Remove(area, range);
        switch (ret.ToCode())
        {
        case Status::Code::GOOD:
            // LOG_VERBOSE(logger, "Removed address range: %u, %u, [%u, %u]", slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
        #if defined(DEBUG)
            // printAddressTable();
        #else
            // CSV 형태로 로그를 만들어서 서버로 전송할 수 있게끔 해줘야 함
        #endif
            // countBufferSize();
            return ret;
        case Status::Code::GOOD_NO_DATA:
            LOG_WARNING(logger, "ADDRESS RANGE TO REMOVE NOT FOUND: %u, %u, [%u, %u]", slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
            return Status(Status::Code::GOOD);
        default:
            LOG_WARNING(logger, "FAILED TO REMOVE: %u, %u, [%u, %u]", slaveID, static_cast<uint8_t>(area), range.GetStartAddress(), range.GetLastAddress());
            return ret;
        }
    }

    void AddressTable::Clear()
    {
        mMapAddressBySlave.clear();
    }

    std::pair<Status, std::set<uint8_t>> AddressTable::RetrieveEntireSlaveID() const
    {
        std::exception exception;
        Status::Code retCode;

        try
        {
            std::set<uint8_t> slaveIDs;

            for (const auto& pair : mMapAddressBySlave)
            {
                slaveIDs.emplace(pair.first);
            }

            return std::make_pair(Status(Status::Code::GOOD), slaveIDs);
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
        return std::make_pair(Status(retCode), std::set<uint8_t>());
    }

    std::pair<Status, Address> AddressTable::RetrieveAddressBySlaveID(const uint8_t slaveID) const
    {
        if (mMapAddressBySlave.find(slaveID) != mMapAddressBySlave.end())
        {
            return std::make_pair(Status(Status::Code::GOOD), mMapAddressBySlave.at(slaveID));
        }
        else
        {
            LOG_WARNING(logger, "ADDRESS WITH SLAVE ID NOT FOUND: %u", slaveID);
            return std::make_pair(Status(Status::Code::BAD_NOT_FOUND), Address());
        }
    }

#if defined(DEBUG)
    void AddressTable::printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %-*s", cellWidth - 2, value);
        strcat(buffer, cell);
    }

    void AddressTable::printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %*u", cellWidth - 4, value);
        strcat(buffer, cell);
    }

    void AddressTable::printAddressTable() const
    {
        const char* dashLine = "----------------------------------------------\n";
        const uint16_t bufferSize = 2048;
        const uint8_t cellWidth = 11;
        
        char buffer[bufferSize];
        uint8_t* castedBuffer = reinterpret_cast<uint8_t*>(buffer);
        memset(buffer, '\0', bufferSize * sizeof(char));
        strcat(buffer, dashLine);
        strcat(buffer, "| Slave  | Area   | Start  | Last   | Qty.   |\n");
        strcat(buffer, dashLine);

        for (const auto& addressBySlave : mMapAddressBySlave)
        {
            const uint8_t  slaveID = addressBySlave.first;
            const Address& address = addressBySlave.second;
            const auto& retrieved  = address.RetrieveArea();

            if (retrieved.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE ADDRESS: %s", retrieved.first.c_str());
                continue;
            }

            for (const auto& area : retrieved.second)
            {
                const auto& ranges = address.RetrieveAddressRange(area);
                const char* strArea = area == jarvis::mb_area_e::COILS          ? "COIL" :
                                      area == jarvis::mb_area_e::DISCRETE_INPUT ? "D.I." :
                                      area == jarvis::mb_area_e::INPUT_REGISTER ? "I.R." : "H.R.";
                
                for (const auto& range : ranges)
                {
                    printCell(cellWidth, slaveID, castedBuffer);
                    printCell(cellWidth, strArea, castedBuffer);
                    printCell(cellWidth, range.GetStartAddress(), castedBuffer);
                    printCell(cellWidth, range.GetLastAddress(), castedBuffer);
                    printCell(cellWidth, range.GetQuantity(), castedBuffer);
                    strcat(buffer, "|\n");
                }
            }
        }

        strcat(buffer, dashLine);
        LOG_INFO(logger, "Modbus Address Table\n%s\n", buffer);
    }
#else
#endif
}}