/**
 * @file AddressTable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브에 대한 주소 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <set>

#include "Address.h"
#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace modbus {

    class AddressTable
    {
    public:
        AddressTable();
        virtual ~AddressTable();
    private:
        using AddressRange = im::NumericAddressRange;
    public:
        Status Update(const uint8_t slaveID, const jvs::node_area_e area, const AddressRange& range);
        Status Remove(const uint8_t slaveID, const jvs::node_area_e area, const AddressRange& range);
        void Clear();
    public:
        std::pair<Status, std::set<uint8_t>> RetrieveEntireSlaveID() const;
        std::pair<Status, Address> RetrieveAddressBySlaveID(const uint8_t slaveID) const;
    private:
    #if defined(DEBUG)
        void printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const;
        void printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const;
        void printAddressTable() const;
    #else
        /**
         * @todo release 빌드 시에는 csv 형태로 로그를 만들어서 서버로 전송해야 함
         */
        void printAddressTable() const;
    #endif

    private:
        std::map<uint8_t, Address> mMapAddressBySlave;
    };
}}