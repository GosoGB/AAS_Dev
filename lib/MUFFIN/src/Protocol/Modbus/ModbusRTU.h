/**
 * @file ModbusRTU.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
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
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Config/Protocol/ModbusRTU.h"
#include "Jarvis/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"



namespace muffin {

    class ModbusRTU
    {
    public:
        ModbusRTU(ModbusRTU const&) = delete;
        void operator=(ModbusRTU const&) = delete;
        static ModbusRTU* CreateInstanceOrNULL();
        static ModbusRTU& GetInstance();
    private:
        ModbusRTU();
        virtual ~ModbusRTU();
    private:
        static ModbusRTU* mInstance;
    
    private:
        using AddressRange = im::NumericAddressRange;
    public:
        Status Config(jarvis::config::ModbusRTU* config, jarvis::config::Rs485* portConfig);
    private:
        SerialConfig convert2SerialConfig(const jarvis::dbit_e dataBit, const jarvis::sbit_e stopBit, const jarvis::pbit_e parityBit);
        Status configurePort(jarvis::prt_e portIndex, jarvis::config::Rs485* portConfig);
        Status addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID);
        // Status removeNodeReference(const uint8_t slaveID, im::Node& node);
    private:
        im::NumericAddressRange createAddressRange(im::Node& node) const;

    public:
        Status Poll();
    private:
        Status implementPolling();
        Status updateVariableNodes();
        Status pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        // Status pollDiscreteInput(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        // Status pollInputRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        // Status pollHoldingRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
    private:
        Status pollCoilTest(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollDiscreteInputTest(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);

    private:
        modbus::NodeTable mNodeTable;
        modbus::AddressTable mAddressTable;
        modbus::PolledDataTable mPolledDataTable;
    };
}