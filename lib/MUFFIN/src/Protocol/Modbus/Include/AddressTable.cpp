/**
 * @file AddressTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-01
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
        : mBufferSize(0)
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
            LOG_VERBOSE(logger, "New slave index has been given");
            Address address;
            address.UpdateAddressMap(area, range);
            mAddressBySlaveMap.emplace(slaveID, address);
            LOG_VERBOSE(logger, "Created the new slave index to the address table");
            return ;
        }

        LOG_VERBOSE(logger, "Found the given slave index from the table");
        it->second.UpdateAddressMap(area, range);
        LOG_VERBOSE(logger, "Updated the slave index from the address table");

        printAddressTable();
        countBufferSize();
    }

    const Status AddressTable::FindSlaveID(const uint8_t slaveID) const
    {
        return mAddressBySlaveMap.find(slaveID) == mAddressBySlaveMap.end() ?
            Status(Status::Code::BAD_NOT_FOUND) :
            Status(Status::Code::GOOD);
    }

    size_t AddressTable::RetrieveBufferSize() const
    {
        return mBufferSize;
    }

    std::set<uint8_t> AddressTable::RetrieveSlaveIdSet() const
    {
        std::set<uint8_t> slaveIdSet;

        for (const auto& address : mAddressBySlaveMap)
        {
            slaveIdSet.emplace(address.first);
        }

        return slaveIdSet;
    }

    const Address& AddressTable::RetrieveAddress(const uint8_t slaveID) const
    {
        ASSERT((mAddressBySlaveMap.find(slaveID) != mAddressBySlaveMap.end()), "ADDRESS WITH SLAVE ID %u NOT FOUND", slaveID);
        return mAddressBySlaveMap.at(slaveID);
    }
    
    void AddressTable::countBufferSize()
    {
        mBufferSize = 0;
        constexpr float MODBUS_REGISTER_SIZE = 16.0f;

        for (const auto& slaveID : mAddressBySlaveMap)
        {
            const auto& addressMap = slaveID.second;

            for (const auto& area : addressMap.RetrieveAreaSet())
            {
                for (const auto& address : addressMap.RetrieveAddressSet(area))
                {
                    if (area == modbus::area_e::COIL || area == modbus::area_e::DISCRETE_INPUT)
                    {
                        const float registerRequired = address.GetQuantity() / MODBUS_REGISTER_SIZE;
                        const uint16_t registerQuantity = static_cast<uint16_t>(0.5f + registerRequired);
                        mBufferSize += registerQuantity;
                    }
                    else
                    {
                        mBufferSize += address.GetQuantity();
                    }
                }
            }
        }
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
        const uint16_t bufferSize = 1024;
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
                const auto& ranges = address.RetrieveAddressSet(area);
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