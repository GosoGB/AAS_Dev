/**
 * @file ModbusRTU.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-10-22
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "IM/Node/NodeStore.h"
#include "Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "ModbusRTU.h"



namespace muffin {

    ModbusRTU* ModbusRTU::CreateInstanceOrNULL()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) ModbusRTU();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MODBUS RTU");
                return mInstance;
            }
        }

        return mInstance;
    }

    ModbusRTU& ModbusRTU::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE EXISTS: CALL FUNCTION \"CreateInstanceOrNULL\" INSTEAD");
        return *mInstance;
    }

    ModbusRTU::ModbusRTU()
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

    Status ModbusRTU::SetPort(jarvis::config::Rs485* portConfig)
    {
        const jarvis::prt_e portIndex = portConfig->GetPortIndex().second;
        Status ret = configurePort(portIndex, portConfig);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE RS-485 PORT: %s", ret.c_str());
            return ret;
        }
        return ret;
    }

    Status ModbusRTU::Config(jarvis::config::ModbusRTU* config)
    {
        const uint8_t slaveID = config->GetSlaveID().second;
        addNodeReferences(slaveID, config->GetNodes().second);
        return Status(Status::Code::GOOD);
    }

    SerialConfig ModbusRTU::convert2SerialConfig(const jarvis::dbit_e dbit, const jarvis::sbit_e sbit, const jarvis::pbit_e pbit)
    {
        const uint8_t dataBits    = (static_cast<uint8_t>(dbit) - 0x05) << 0x2;
        const uint8_t stopBits    = (sbit == jarvis::sbit_e::SBIT_1 ? 0x01 : 0x03) << 0x4;        
        const uint8_t parityBits  = pbit == jarvis::pbit_e::NONE ? 0x00 : 
                                    pbit == jarvis::pbit_e::EVEN ? 0x02 :
                                    0x03;

        const uint32_t config = 0x8000000 | dataBits | stopBits | parityBits;
        LOG_DEBUG(logger, "Serial Config: 0x%X", config);
        LOG_DEBUG(logger, "Serial Config: %s", config == SERIAL_8N1 ? "SERIAL_8N1" : "INVALID!");
        return static_cast<SerialConfig>(config);
    }

    Status ModbusRTU::configurePort(jarvis::prt_e portIndex, jarvis::config::Rs485* portConfig)
    {
        if (portIndex == jarvis::prt_e::PORT_2)
        {
            const jarvis::bdr_e baudrate   = portConfig->GetBaudRate().second;
            const jarvis::dbit_e dataBit   = portConfig->GetDataBit().second;
            const jarvis::pbit_e parityBit = portConfig->GetParityBit().second;
            const jarvis::sbit_e stopBit   = portConfig->GetStopBit().second;

            SerialConfig serialConfig = convert2SerialConfig(dataBit, stopBit, parityBit);
            ModbusRTUClient.begin(*RS485, Convert.ToUInt32(baudrate), serialConfig);
            return Status(Status::Code::GOOD);
        }
        else
        {
            ASSERT(false, "UNDEFINED OR UNSUPPORTED CONFIGURATION");
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }
    }

    Status ModbusRTU::addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID)
    {
        Status ret(Status::Code::UNCERTAIN);
        im::NodeStore& nodeStore = im::NodeStore::GetInstance();

        for (auto& nodeID : vectorNodeID)
        {
            const auto result = nodeStore.GetNodeReference(nodeID);
            if (result.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO GET A REFERENCE TO NODE: %s", nodeID.c_str())
                return result.first;
            }
            
            im::Node* reference = result.second;
            ret = mNodeTable.Update(slaveID, *reference);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE NODE RERENCE TABLE: %s", nodeID.c_str());
                return ret;
            }

            const jarvis::mb_area_e area = reference->VariableNode.GetModbusArea();
            const AddressRange range = createAddressRange(*reference);

            ret = mAddressTable.Update(slaveID, area, range);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS TABLE: %s", ret.c_str());
                return Status(Status::Code::BAD);
            }
            
            LOG_DEBUG(logger, "Updated node reference: %s", nodeID.c_str());
        }

        return Status(Status::Code::GOOD);
    }

/** * @brief 향후 개발 예정입니다. --> Status ModbusRTU::RemoveReferece(const uint8_t slaveID, im::Node& node)
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
 */


    /**
     * @todo node 개체 말고 필요한 정보만 매개변수로 넘기도록 수정해야 합니다.
     * @author 김주성
     */
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
                    break;
                case jarvis::mb_area_e::DISCRETE_INPUT:
                    ret = pollDiscreteInput(slaveID, addressSetToPoll);
                    break;
                case jarvis::mb_area_e::INPUT_REGISTER:
                    ret = pollInputRegister(slaveID, addressSetToPoll);
                    break;
                case jarvis::mb_area_e::HOLDING_REGISTER:
                    ret = pollHoldingRegister(slaveID, addressSetToPoll);
                    break;
                default:
                    ASSERT(false, "UNDEFINED MODBUS MEMORY AREA");
                    break;
                }

                if (ret != Status(Status::Code::GOOD))
                {
                    LOG_ERROR(logger, "FAILED TO POLL: %s, SlaveID : %u, AREA : %u", ret.c_str(), slaveID, static_cast<uint8_t>(area));
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::updateVariableNodes()
    {
        Status ret(Status::Code::UNCERTAIN);

        const auto retrievedSlaveInfo = std::move(mNodeTable.RetrieveEntireSlaveID());
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
                const uint16_t address  = node->VariableNode.GetAddress().Numeric;
                const uint16_t quantity = node->VariableNode.GetQuantity();
                const jarvis::mb_area_e area = node->VariableNode.GetModbusArea();

                modbus::datum_t datum;
                datum.Address = address;
                datum.Value = 0;
                datum.IsOK = false;

                std::vector<modbus::datum_t> vectorDatum;
                im::poll_data_t polledData;
                polledData.AddressType = jarvis::adtp_e::NUMERIC;
                polledData.Address.Numeric = address;
                polledData.Timestamp = timestampInMillis;

                /**
                 * @todo ModbusRTU 클래스에서 얻은 상태 코드에 따라서 MUFFIN 상태 코드로 변환하는 작업이 필요합니다.
                 *       현재는 시간 상 IsOK로 Boolean 값을 받지만 향후에는 MUFFIN Status::Code로 변환해야 합니다.
                 */
                switch (area)
                {
                case jarvis::mb_area_e::COILS:
                    datum = mPolledDataTable.RetrieveCoil(slaveID, address);
                    goto BIT_MEMORY;
                case jarvis::mb_area_e::DISCRETE_INPUT:
                    datum = mPolledDataTable.RetrieveDiscreteInput(slaveID, address);
                BIT_MEMORY:
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
                    vectorDatum.reserve(quantity);
                    for (size_t i = 0; i < quantity; ++i)
                    {
                        vectorDatum.emplace_back(mPolledDataTable.RetrieveInputRegister(slaveID, address + i));
                    }
                    LOG_INFO(logger, "vectorDatum size : %d value : %d", vectorDatum.size(), vectorDatum[0].Value);
                    goto REGISTER_MEMORY;
                case jarvis::mb_area_e::HOLDING_REGISTER:
                    vectorDatum.reserve(quantity);
                    for (size_t i = 0; i < quantity; ++i)
                    {
                        vectorDatum.emplace_back(mPolledDataTable.RetrieveHoldingRegister(slaveID, address + i));
                    }
                REGISTER_MEMORY:
                    if (datum.IsOK == false)
                    {
                        polledData.StatusCode = Status::Code::BAD;
                        polledData.Value.UInt16 =  vectorDatum[0].Value;
                    }
                    else
                    {
                        polledData.StatusCode = Status::Code::GOOD;
                        polledData.Value.UInt16 =  vectorDatum[0].Value;
                    }
                    polledData.ValueType = jarvis::dt_e::UINT16;
                    node->VariableNode.Update(polledData);
                    break;
                default:
                    break;
                }
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();

            ModbusRTUClient.requestFrom(slaveID, COILS, startAddress, pollQuantity);
            delay(80);
            const char* lastError = ModbusRTUClient.lastError();
            ModbusRTUClient.clearError();

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
                    LOG_DEBUG(logger, "[COIL] read value : %d, address : %d ", value, address);
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

    Status ModbusRTU::pollDiscreteInput(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            ModbusRTUClient.requestFrom(slaveID, DISCRETE_INPUTS, startAddress, pollQuantity);
            delay(80);
            const char* lastError = ModbusRTUClient.lastError();
            ModbusRTUClient.clearError();

            if (lastError != nullptr)
            {
                LOG_ERROR(logger, "FAILED TO POLL: %s", lastError);
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateDiscreteInput(slaveID, address, INVALID_VALUE);
                }
                continue;
            }
            // LOG_VERBOSE(logger, "Poll: %u bits", pollQuantity);


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = ModbusRTUClient.read();
                // LOG_DEBUG(logger, "[DISCRETE INPUT] read value : %d, address : %d ", value, address);
                switch (value)
                {
                case 1:
                case 0:
                    mPolledDataTable.UpdateDiscreteInput(slaveID, address, value);
                    continue;
                default:
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateDiscreteInput(slaveID, address, INVALID_VALUE);
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::pollInputRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            // LOG_INFO(logger,"[INPUT REGISTERS] slaveID : %d, startAddress : %d , pollQuantity : %d", slaveID, startAddress, pollQuantity);
            ModbusRTUClient.requestFrom(slaveID, INPUT_REGISTERS, startAddress, pollQuantity);
            delay(80);
            const char* lastError = ModbusRTUClient.lastError();
            ModbusRTUClient.clearError();

            if (lastError != nullptr)
            {
                LOG_ERROR(logger, "[INPUT REGISTERS] FAILED TO POLL: %s", lastError);
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateInputRegister(slaveID, address, INVALID_VALUE);
                }
                continue;
            }
            // LOG_VERBOSE(logger, "Poll: %u bits", pollQuantity);


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int32_t value = ModbusRTUClient.read();
                // LOG_DEBUG(logger, "[INPUT REGISTER] read value : %d, address : %d ", value, address);
                if (value == -1)
                {
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateInputRegister(slaveID, address, INVALID_VALUE);
                    return ret;
                }
                else
                {
                    mPolledDataTable.UpdateInputRegister(slaveID, address, (uint16_t)value);
                }
            }
        }

        return ret;
    }

    Status ModbusRTU::pollHoldingRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            ModbusRTUClient.requestFrom(slaveID, HOLDING_REGISTERS, startAddress, pollQuantity);
            delay(80);
            const char* lastError = ModbusRTUClient.lastError();
            ModbusRTUClient.clearError();

            if (lastError != nullptr)
            {
                LOG_ERROR(logger, "[HOLDING REGISTERS] FAILED TO POLL: %s", lastError);
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateHoldingRegister(slaveID, address, INVALID_VALUE);
                }
                continue;
            }
            LOG_VERBOSE(logger, "Poll: %u bits", pollQuantity);


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int32_t value = ModbusRTUClient.read();
                LOG_DEBUG(logger, "[HOLDING REGISTER] read value : %d, address : %d ", value, address);
                if (value == -1)
                {
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateHoldingRegister(slaveID, address, INVALID_VALUE);
                    return ret;
                }
                else
                {
                    mPolledDataTable.UpdateHoldingRegister(slaveID, address, (uint16_t)value);
                }
            }
        }

        return ret;
    }


    ModbusRTU* ModbusRTU::mInstance = nullptr;
}