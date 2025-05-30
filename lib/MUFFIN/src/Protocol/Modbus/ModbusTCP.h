/**
 * @file ModbusTCP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-10-22
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
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Config/Protocol/ModbusTCP.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"
#include "Protocol/Modbus/Include/ArduinoModbus/src/ModbusTCPClient.h"
#if defined(MT11)
    #include "Network/Ethernet/W5500/EthernetClient.h"
#else
    #include "WiFi.h"
#endif


namespace muffin {

    class ModbusTCP
    {
    public:
    #if defined(MT11)
        ModbusTCP(W5500& interface, const w5500::sock_id_e sock_id);
    #else
        ModbusTCP();
    #endif
        virtual ~ModbusTCP();
    private:
        using AddressRange = im::NumericAddressRange;
    public:
        Status Config(jvs::config::ModbusTCP* config);
        void Clear();
        void SetTimeoutError();
    public:
        IPAddress GetServerIP();
        uint16_t GetServerPort();
    private:
        Status addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID);
        // Status removeNodeReference(const uint8_t slaveID, im::Node& node);
    private:
        im::NumericAddressRange createAddressRange(const uint16_t address, const uint16_t quantity) const;

    public:
        Status Poll();
        modbus::datum_t GetAddressValue(const uint8_t slaveID, const uint16_t address, const jvs::node_area_e area);
    private:
        Status implementPolling();
        Status updateVariableNodes();
        Status pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollDiscreteInput(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollInputRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);
        Status pollHoldingRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet);

    private:
        modbus::NodeTable mNodeTable;
        modbus::AddressTable mAddressTable;
        modbus::PolledDataTable mPolledDataTable;
    
    private:
        IPAddress mServerIP;
        uint16_t mServerPort;
    public:
    #if defined(MT11)
        w5500::EthernetClient* mClient = nullptr;
    #else
        WiFiClient mClient;
    #endif
    ModbusTCPClient* mModbusTCPClient = nullptr;
    };
}