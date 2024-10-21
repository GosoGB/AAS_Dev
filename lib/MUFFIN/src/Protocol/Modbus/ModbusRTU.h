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

#include "Include/ArduinoRS485/src/RS485.h"
#include "Common/Status.h"
#include "Include/AddressTable.h"
#include "Include/NodeTable.h"
#include "Include/PolledDataTable.h"
#include "Include/TypeDefinitions.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin {

    class ModbusRTU
    {
    public:
        ModbusRTU();
        virtual ~ModbusRTU();
    private:
        using AddressRange = im::NumericAddressRange;
    
    public:
        void InitTest();
        // Status Config(jarvis::config::Base* config);
        void SetPort(HardwareSerial& port);
        Status AddNodeReference(const uint8_t slaveID, im::Node& node);
        Status RemoveReferece(const uint8_t slaveID, im::Node& node);
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
        RS485Class* mRS485;
        modbus::NodeTable mNodeTable;
        modbus::AddressTable mAddressTable;
        modbus::PolledDataTable mPolledDataTable;
    private:
        static constexpr uint8_t DE_PIN_NUMBER =  4;
        static constexpr uint8_t RE_PIN_NUMBER =  5;
        static constexpr uint8_t TX_PIN_NUMBER = 17;
    };
}