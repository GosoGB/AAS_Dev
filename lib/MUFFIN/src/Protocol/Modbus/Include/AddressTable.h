/**
 * @file AddressTable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜의 주소 테이블을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>

#include "Address.h"



namespace muffin { namespace modbus {

    class AddressTable
    {
    public:
        AddressTable();
        virtual ~AddressTable();
    public:
        void UpdateAddressTable(const uint8_t slaveID, const area_e area, const muffin::im::NumericAddressRange& range);
    private:
        std::map<uint8_t, Address> mAddressBySlaveMap;
    };
}}