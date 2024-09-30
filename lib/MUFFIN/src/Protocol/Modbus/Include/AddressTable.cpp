/**
 * @file AddressTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜의 주소 테이블을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




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
            Address address;
            address.UpdateAddressMap(area, range);
            mAddressBySlaveMap.emplace(slaveID, address);
            return ;
        }

        it->second.UpdateAddressMap(area, range);
    }
}}