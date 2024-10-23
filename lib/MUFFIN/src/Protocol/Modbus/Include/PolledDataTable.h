/**
 * @file PolledDataTable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-22
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>

#include "Common/Status.h"
#include "PolledData.h"



namespace muffin { namespace modbus {

    class PolledDataTable
    {
    public:
        PolledDataTable();
        virtual ~PolledDataTable();
    public:
        Status UpdateCoil(const uint8_t slaveID, const uint16_t address, const int8_t value);
        Status UpdateDiscreteInput(const uint8_t slaveID, const uint16_t address, const int8_t value);
        Status UpdateInputRegister(const uint8_t slaveID, const uint16_t address, const int32_t value);
        Status UpdateHoldingRegister(const uint8_t slaveID, const uint16_t address, const int32_t value);
    public:
        datum_t RetrieveCoil(const uint8_t slaveID, const uint16_t address) const;
        datum_t RetrieveDiscreteInput(const uint8_t slaveID, const uint16_t address) const;
        datum_t RetrieveInputRegister(const uint8_t slaveID, const uint16_t address) const;
        datum_t RetrieveHoldingRegister(const uint8_t slaveID, const uint16_t address) const;
    private:
        std::map<uint8_t, PolledData> mMapPolledDataBySlave;
    };
}}