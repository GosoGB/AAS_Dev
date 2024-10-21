/**
 * @file ModbusRTU.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "ModbusRTU.h"



namespace muffin {

    ModbusRTU::ModbusRTU()
        : mRS485(nullptr)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ModbusRTU::~ModbusRTU()
    {
        if (mRS485 != nullptr)
        {
            delete mRS485;
            mRS485 = nullptr;
            LOG_VERBOSE(logger, "Destroyed RS485 instance at address: %p");
        }
        
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void ModbusRTU::InitTest()
    {
        // ModbusRTUClient.begin(*mRS485, 9600, SERIAL_8N1);
    }
    
    void ModbusRTU::SetPort(HardwareSerial& port)
    {
        mRS485 = new RS485Class(port, TX_PIN_NUMBER, DE_PIN_NUMBER, RE_PIN_NUMBER);
        ModbusRTUClient.begin(*mRS485, 9600, SERIAL_8N1);
    }

    Status ModbusRTU::AddNodeReference(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Update(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE NODE REFERENCE: %s", ret.c_str());
            return Status(Status::Code::BAD);
        }

        const jarvis::mb_area_e area = node.VariableNode.GetModbusArea();
        const AddressRange range = createAddressRange(node);

        ret = mAddressTable.Update(slaveID, area, range);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS TABLE: %s", ret.c_str());
            return Status(Status::Code::BAD);
        }
        
        LOG_VERBOSE(logger, "Updated node reference: %s", node.GetNodeID().c_str());
        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::RemoveReferece(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Remove(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REMOVE NODE REFERENCE: %s", ret.c_str());
            return Status(Status::Code::BAD);
        }

        const jarvis::mb_area_e area = node.VariableNode.GetModbusArea();
        const AddressRange range = createAddressRange(node);

        ret = mAddressTable.Remove(slaveID, area, range);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REMOVE ADDRESS TABLE: %s", ret.c_str());
            return Status(Status::Code::BAD);
        }
        
        LOG_VERBOSE(logger, "Removed node reference: %s", node.GetNodeID().c_str());
        return Status(Status::Code::GOOD);
    }

    im::NumericAddressRange ModbusRTU::createAddressRange(im::Node& node) const
    {
        const uint16_t address  = node.VariableNode.GetAddress().Numeric;
        const uint16_t quantity = node.VariableNode.GetQuantity();
        return AddressRange(address, quantity);
    }

    Status ModbusRTU::Poll()
    {
        Status ret = implementPolling();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
        }
        
        ret = updateVariableNodes();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE NODES: %s", ret.c_str());
        }

        return ret;
    }

    Status ModbusRTU::implementPolling()
    {
        Status ret(Status::Code::UNCERTAIN);

        const auto retrievedSlaveInfo = mAddressTable.RetrieveEntireSlaveID();
        if (retrievedSlaveInfo.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SLAVE ID FOR POLLING: %s", retrievedSlaveInfo.first.c_str());
            return Status(Status::Code::BAD);
        }

        for (const auto& slaveID : retrievedSlaveInfo.second)
        {
            const auto retrievedAddressInfo = mAddressTable.RetrieveAddressBySlaveID(slaveID);
            if (retrievedAddressInfo.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE ADDRESSES FOR POLLING: %s", retrievedAddressInfo.first.c_str());
                return Status(Status::Code::BAD);
            }

            const auto retrievedAreaInfo = retrievedAddressInfo.second.RetrieveArea();
            if (retrievedAreaInfo.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE MODBUS AREA FOR POLLING: %s", retrievedAreaInfo.first.c_str());
                return Status(Status::Code::BAD);
            }

            for (const auto& area : retrievedAreaInfo.second)
            {
                const auto& addressSetToPoll = retrievedAddressInfo.second.RetrieveAddressRange(area);
                switch (area)
                {
                case jarvis::mb_area_e::COILS:
                    ret = pollCoil(slaveID, addressSetToPoll);
                    // ret = pollCoilTest(slaveID, addressSetToPoll);
                    break;
                // case jarvis::mb_area_e::DISCRETE_INPUT:
                //     ret = pollDiscreteInput(slaveID, addressSetToPoll);
                //     break;
                // case jarvis::mb_area_e::INPUT_REGISTER:
                //     ret = pollInputRegister(slaveID, addressSetToPoll);
                //     break;
                // case jarvis::mb_area_e::HOLDING_REGISTER:
                //     ret = pollHoldingRegister(slaveID, addressSetToPoll);
                //     break;
                default:
                    ASSERT(false, "UNDEFINED MODBUS MEMORY AREA");
                    break;
                }

                if (ret != Status(Status::Code::GOOD))
                {
                    LOG_ERROR(logger, "FAILED TO POLL: %s, %u, %u", ret.c_str(), slaveID, static_cast<uint8_t>(area));
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::updateVariableNodes()
    {
        Status ret(Status::Code::UNCERTAIN);

        const auto retrievedSlaveInfo = mNodeTable.RetrieveEntireSlaveID();
        if (retrievedSlaveInfo.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SLAVE ID FOR NODE UPDATE: %s", retrievedSlaveInfo.first.c_str());
            return Status(Status::Code::BAD_NOT_FOUND);
        }

        const uint64_t timestampInMillis = GetTimestampInMillis();
        for (const auto& slaveID : retrievedSlaveInfo.second)
        {
            const auto retrievedNodeInfo = mNodeTable.RetrieveNodeBySlaveID(slaveID);
            if (retrievedNodeInfo.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE NODE REFERENCES: %s", retrievedNodeInfo.first.c_str());
                return Status(Status::Code::BAD_NOT_FOUND);
            }

            for (auto& node : retrievedNodeInfo.second)
            {
                jarvis::mb_area_e area = node->VariableNode.GetModbusArea();
                const uint16_t address = node->VariableNode.GetAddress().Numeric;
                const uint16_t quantity = node->VariableNode.GetQuantity();

                modbus::datum_t datum;
                im::poll_data_t polledData;
                polledData.AddressType = jarvis::adtp_e::NUMERIC;
                polledData.Address.Numeric = address;
                polledData.Timestamp = timestampInMillis;

                /**
                 * @todo ModbusRTU 클래스에서 얻은 상태 코드에 따라서 
                 *       MUFFIN 상태 코드로 변환하는 작업이 필요합니다.
                 */
                switch (area)
                {
                case jarvis::mb_area_e::COILS:
                case jarvis::mb_area_e::DISCRETE_INPUT:
                    datum = mPolledDataTable.RetrieveCoil(slaveID, address);
                    if (datum.IsOK == false)
                    {
                        polledData.StatusCode = Status::Code::BAD;
                        polledData.Value.Boolean = datum.Value == 1 ? true : false;
                    }
                    else
                    {
                        polledData.StatusCode = Status::Code::GOOD;
                        polledData.Value.Boolean = datum.Value == 1 ? true : false;
                    }
                    polledData.ValueType = jarvis::dt_e::BOOLEAN;
                    node->VariableNode.Update(polledData);
                    break;
                case jarvis::mb_area_e::INPUT_REGISTER:
                    /* code */
                    break;
                case jarvis::mb_area_e::HOLDING_REGISTER:
                    /* code */
                    break;
                default:
                    break;
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            LOG_INFO(logger,"slaveID : %d, startAddress : %d , pollQuantity : %d", slaveID, startAddress, pollQuantity);
            int pollResult = ModbusRTUClient.requestFrom(slaveID, DISCRETE_INPUTS, startAddress, pollQuantity);
     
            const char* lastError = ModbusRTUClient.lastError();

            if (lastError != nullptr)
            {
                LOG_ERROR(logger, "FAILED TO POLL: %s", lastError);
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateCoil(slaveID, address, INVALID_VALUE);
                }
                continue;
            }
            LOG_VERBOSE(logger, "Poll: %u bits", pollQuantity);

            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = ModbusRTUClient.read();

                switch (value)
                {
                case 1:
                case 0:
                    mPolledDataTable.UpdateCoil(slaveID, address, value);
                    continue;
                default:
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateCoil(slaveID, address, INVALID_VALUE);
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::pollCoilTest(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();

            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = random(0, 2);

                switch (value)
                {
                case 1:
                case 0:
                    mPolledDataTable.UpdateCoil(slaveID, address, value);
                    continue;
                default:
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    mPolledDataTable.UpdateCoil(slaveID, address, -1);
                }
            }
        }

        return Status(Status::Code::GOOD);
    }
}