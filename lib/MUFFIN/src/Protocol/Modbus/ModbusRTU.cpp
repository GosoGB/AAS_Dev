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




#include <ArduinoRS485.h>
#include <ArduinoModbus.h>
#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusRTU.h"



namespace muffin {

    ModbusRTU::ModbusRTU()
        : mSerial(nullptr)
        , mPolledDataIndex(0)
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
        mSerial = &port;
    }

    Status ModbusRTU::AddNodeReference(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Update(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        const uint16_t address    = node.VariableNode.GetAddress();
        const uint16_t quantity   = node.VariableNode.GetQuantity();
        const modbus::area_e area = node.VariableNode.GetModbusArea();
        im::NumericAddressRange range(address, quantity);

        mAddressTable.UpdateAddressTable(slaveID, area, range);
        const size_t bufferSize = mAddressTable.RetrieveBufferSize();
        mPolledData.reserve(bufferSize);
        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::RemoveReferece(const uint8_t slaveID, const std::string& nodeID)
    {
        for (auto it = mNodeReferences.begin(); it != mNodeReferences.end(); ++it)
        {
            if ((*it)->GetNodeID() == nodeID)
            {
                mNodeReferences.erase(it);
                LOG_VERBOSE(muffin::logger, "Removed node: %s", nodeID.c_str());
                return Status(Status::Code::GOOD);
            }
        }
        
        LOG_ERROR(muffin::logger, "FAILED TO REMOVE REFERENCE TO NODE \"%s\" SINCE IT'S NOT FOUND");
        return Status(Status::Code::BAD_NOT_FOUND);
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

    Status pollCoil(const uint8_t slaveID, const std::set<muffin::im::NumericAddressRange>& addressSet)
    {
        if (addressSet.size() == 1)
        {
            const auto it = addressSet.begin();
            const uint16_t address  = it->GetStartAddress();
            const uint16_t quantity = it->GetQuantity();

            if (ModbusRTUClient.requestFrom(slaveID, COILS, address, quantity) != quantity)
            {
                LOG_ERROR(logger, "FAILED TO POLL COIL: %s", ModbusRTUClient.lastError());
                return Status(Status::Code::BAD);
            }
            LOG_VERBOSE(logger, "Succeeded to poll %u bits from Coil block", quantity);
            
            const uint16_t reps = static_cast<uint16_t>(0.5f + (quantity / 16.0f));
            for (size_t i = 0; i < reps; i++)
            {
                modbus::datum_t datum;
                datum.SlaveID = slaveID;
                datum.Address = address + i;
                datum.Area = modbus::area_e::COIL;

                uint8_t bitIndex = 0;
                
                while (ModbusRTUClient.available())
                {
                    int8_t value = ModbusRTUClient.read();

                    if (value == 1)
                    {
                        datum.Value |= (1 << bitIndex);
                    }
                    else if (value == 0)
                    {
                        datum.Value &= ~(1 << bitIndex);
                    }
                    else
                    {
                        return Status(Status::Code::BAD_DATA_UNAVAILABLE);
                    }
                    
                    datum.Address = address;
                }
            }
            
            return Status(Status::Code::GOOD);
        }
        
        for (auto it = addressSet.begin(); it != std::prev(addressSet.end()); ++it)
        {
            address.GetStartAddress();
        }
        
        
        
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}