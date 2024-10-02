/**
 * @file ModbusRTU.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-10-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "ModbusRTU.h"



namespace muffin {

    ModbusRTU::ModbusRTU()
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        : mRS485(Serial2, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN)
    #endif
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ModbusRTU::~ModbusRTU()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    void ModbusRTU::SetPort(HardwareSerial& port)
    {
    #if not defined(MODLINK_L) && not defined(MODLINK_ML10)
        mRS485(SERIAL_PORT_HARDWARE, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN);
        mSerial = &port;
    #endif
    }

    Status ModbusRTU::AddNodeReference(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Update(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        const modbus::area_e area = node.VariableNode.GetModbusArea();
        const uint16_t address    = node.VariableNode.GetAddress();
        const uint16_t quantity   = node.VariableNode.GetQuantity();
        im::NumericAddressRange range(address, quantity);

        mAddressTable.Update(slaveID, area, range);
        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::RemoveReferece(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Remove(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        const modbus::area_e area = node.VariableNode.GetModbusArea();
        const uint16_t address    = node.VariableNode.GetAddress();
        const uint16_t quantity   = node.VariableNode.GetQuantity();
        im::NumericAddressRange range(address, quantity);

        mAddressTable.Remove(slaveID, area, range);
        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::Poll()
    {
        for (const auto& slaveID : mAddressTable.RetrieveSlaveIdSet())
        {
            const modbus::Address address = mAddressTable.RetrieveAddress(slaveID);

            for (const auto& area : address.RetrieveAreaSet())
            {
                const auto& addressSet = address.RetrieveAddressSet(area);

                switch (area)
                {
                case modbus::area_e::COIL:
                    pollCoil(slaveID, addressSet);
                    break;
                case modbus::area_e::DISCRETE_INPUT:
                    pollDiscreteInput(slaveID, addressSet);
                    break;
                case modbus::area_e::INPUT_REGISTER:
                    pollInputRegister(slaveID, addressSet);
                    break;
                case modbus::area_e::HOLDING_REGISTER:
                    pollHoldingRegister(slaveID, addressSet);
                    break;
                default:
                    break;
                }

                // for (const auto& range : addressSet)
                // {
                //     const uint16_t address  = range.GetStartAddress();
                //     const uint16_t quantity = range.GetQuantity();
                // }
            }
        }
        
        for (const auto& slaveID : mAddressTable.RetrieveSlaveIdSet())
        {
            for (auto& node : mNodeTable.RetrieveBySlaveID(slaveID))
            {
                // node->VariableNode.UpdateData()
            }
        }

        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status ModbusRTU::pollCoil(const uint8_t slaveID, const std::set<muffin::im::NumericAddressRange>& addressSet)
    {
        for (auto it = addressSet.begin(); it != addressSet.end(); ++it)
        {
            const uint16_t startAddress  = it->GetStartAddress();
            const uint16_t quantity = it->GetQuantity();

            if (ModbusRTUClient.requestFrom(slaveID, COILS, startAddress, quantity) != quantity)
            {
                LOG_ERROR(logger, "FAILED TO POLL COIL: %s", ModbusRTUClient.lastError());
                return Status(Status::Code::BAD);
            }
            ASSERT((ModbusRTUClient.available() == quantity), "Quantity and the available bits do not match");
            LOG_VERBOSE(logger, "Succeeded to poll %u bits from Coil block", quantity);

            for (size_t i = 0; i < quantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = ModbusRTUClient.read();

                switch (value)
                {
                case true:
                case false:
                    mPolledDataTable.UpdateCoil(slaveID, address, value);
                    continue;
                default:
                    return Status(Status::Code::BAD_DATA_UNAVAILABLE);
                }
            }
        }

        return Status(Status::Code::GOOD);
    }
}