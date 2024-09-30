/**
 * @file AddressTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜의 주소 테이블을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-30
 * @version 0.0.1
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
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    AddressTable::~AddressTable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void AddressTable::UpdateAddressTable(const uint8_t slaveID, const area_e area, const muffin::im::NumericAddressRange& range)
    {
        auto it = mAddressBySlaveMap.find(slaveID);
        if (it == mAddressBySlaveMap.end())
        {
            LOG_DEBUG(logger, "New slave index has been given");
            Address address;
            address.UpdateAddressMap(area, range);
            mAddressBySlaveMap.emplace(slaveID, address);
            LOG_DEBUG(logger, "Created the new slave index to the address table");
            return ;
        }

        LOG_DEBUG(logger, "Found the given slave index from the table");
        it->second.UpdateAddressMap(area, range);
        LOG_DEBUG(logger, "Updated the slave index from the address table");
        printAddressTable();
    }

    const Status AddressTable::FindSlaveID(const uint8_t slaveID) const
    {
        return mAddressBySlaveMap.find(slaveID) == mAddressBySlaveMap.end() ?
            Status(Status::Code::BAD_NOT_FOUND) :
            Status(Status::Code::GOOD);
    }

    const Address& AddressTable::RetrieveBySlaveID(const uint8_t slaveID) const
    {
        ASSERT((mAddressBySlaveMap.find(slaveID) != mAddressBySlaveMap.end()), "ADDRESS WITH SLAVE ID %u NOT FOUND", slaveID);
        return mAddressBySlaveMap.at(slaveID);
    }   
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
        const uint16_t bufferSize = 512;
        const uint8_t cellWidth = 11;
        
        char buffer[bufferSize];
        uint8_t* castedBuffer = reinterpret_cast<uint8_t*>(buffer);
        memset(buffer, '\0', bufferSize * sizeof(char));
        strcat(buffer, dashLine);
        strcat(buffer, "| Slave  | Area   | Start  | Last   | Qty.   |\n");
        strcat(buffer, dashLine);

        for (const auto& addressBySlave : mAddressBySlaveMap)
        {
            const auto& slaveID = addressBySlave.first;
            const auto& address = addressBySlave.second;
            const auto& areaSet = address.RetrieveAreaSet();

            for (const auto& area : areaSet)
            {
                const auto& ranges = address.RetrieveByArea(area);
                const char* strArea = area == area_e::COIL             ? "COIL" :
                                        area == area_e::DISCRETE_INPUT ? "D.I." :
                                        area == area_e::INPUT_REGISTER ? "I.R." : "H.R.";
                
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
}}