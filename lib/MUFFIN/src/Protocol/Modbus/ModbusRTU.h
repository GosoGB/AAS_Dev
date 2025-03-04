/**
 * @file ModbusRTU.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-10-22
 * @version 1.0.0
 * 
 * @todo 김병우 수석께 DE/RE 핀 번호가 4/5번인지 확인을 받아야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Include/AddressTable.h"
#include "Include/NodeTable.h"
#include "Include/PolledDataTable.h"
#include "Include/TypeDefinitions.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"
#include "Protocol/SPEAR/Include/TypeDefinitions.h"



namespace muffin {

    class ModbusRTU
    {
    public:
        ModbusRTU();
        virtual ~ModbusRTU();
    private:
        using AddressRange = im::NumericAddressRange;
    public:
        Status SetPort(jvs::config::Rs485* portConfig);
        Status Config(jvs::config::ModbusRTU* config);
        void Clear();
    private:
        SerialConfig convert2SerialConfig(const jvs::dbit_e dataBit, const jvs::sbit_e stopBit, const jvs::pbit_e parityBit);
        Status configurePort(jvs::prt_e portIndex, jvs::config::Rs485* portConfig);
        Status addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID);
        // Status removeNodeReference(const uint8_t slaveID, im::Node& node);
    private:
        im::NumericAddressRange createAddressRange(const uint16_t address, const uint16_t quantity) const;

    public:
        Status Poll();
        Status PollTemp();
        modbus::datum_t GetAddressValue(const uint8_t slaveID, const uint16_t address, const jvs::mb_area_e area);
    private:
        Status implementPolling();
        Status updateVariableNodes();
        Status pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollDiscreteInput(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollInputRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollHoldingRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
    
    public:
        modbus::AddressTable mAddressTable;
        jvs::prt_e mPort;
    private:
        modbus::NodeTable mNodeTable;
        modbus::PolledDataTable mPolledDataTable;
    };
}