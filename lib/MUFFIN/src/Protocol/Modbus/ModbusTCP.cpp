/**
 * @file ModbusTCP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 클래스를 정의합니다.
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
#include "Include/ArduinoModbus/src/ModbusTCPClient.h"
#include "ModbusTCP.h"
#include "ModbusMutex.h"


namespace muffin {


#if defined(MT11)
    ModbusTCP::ModbusTCP(W5500& interface, const w5500::sock_id_e sock_id)
    {
        if (sock_id != w5500::sock_id_e::SOCKET_0)
        {
            mClient = new w5500::EthernetClient(interface, sock_id);
            mModbusTCPClient = new ModbusTCPClient(*mClient);
        }
        
    }
#else
    ModbusTCP::ModbusTCP()
    {
        mModbusTCPClient = new ModbusTCPClient(mClient);
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
#endif
    
    ModbusTCP::~ModbusTCP()
    {   
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status ModbusTCP::Config(jvs::config::ModbusTCP* config)
    {
        addNodeReferences(config->GetSlaveID().second, config->GetNodes().second);
        mServerIP   = config->GetIPv4().second;
        mServerPort = config->GetPort().second;
        return Status(Status::Code::GOOD);
    }

    void ModbusTCP::Clear()
    {
        mNodeTable.Clear();
        mAddressTable.Clear();
        mPolledDataTable.Clear();
    }

    Status ModbusTCP::addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID)
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

            const jvs::node_area_e area = reference->VariableNode.GetNodeArea();
            const AddressRange range = createAddressRange(reference->VariableNode.GetAddress().Numeric, reference->VariableNode.GetQuantity());
    
            ret = mAddressTable.Update(slaveID, area, range);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS TABLE: %s", ret.c_str());
                return Status(Status::Code::BAD);
            }
        }

        return Status(Status::Code::GOOD);
    }

/** * @brief 향후 개발 예정입니다. --> Status ModbusTCP::RemoveReferece(const uint8_t slaveID, im::Node& node)
     Status ModbusTCP::RemoveReferece(const uint8_t slaveID, im::Node& node)
    {
        Status ret = mNodeTable.Remove(slaveID, node);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REMOVE NODE REFERENCE: %s", ret.c_str());
            return Status(Status::Code::BAD);
        }

        const jvs::node_area_e area = node.VariableNode.GetNodeArea();
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

    im::NumericAddressRange ModbusTCP::createAddressRange(const uint16_t address, const uint16_t quantity) const
    {
        return AddressRange(address, quantity);
    }

    Status ModbusTCP::Poll()
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

    Status ModbusTCP::implementPolling()
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
                case jvs::node_area_e::COILS:
                    ret = pollCoil(slaveID, addressSetToPoll);
                    break;
                case jvs::node_area_e::DISCRETE_INPUT:
                    ret = pollDiscreteInput(slaveID, addressSetToPoll);
                    break;
                case jvs::node_area_e::INPUT_REGISTER:
                    ret = pollInputRegister(slaveID, addressSetToPoll);
                    break;
                case jvs::node_area_e::HOLDING_REGISTER:
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

    Status ModbusTCP::updateVariableNodes()
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
                const jvs::node_area_e area = node->VariableNode.GetNodeArea();

                modbus::datum_t datum;
                datum.Address = address;
                datum.Value = 0;
                datum.IsOK = false;

                std::vector<im::poll_data_t> vectorPolledData;
                im::poll_data_t polledData;
                polledData.StatusCode = Status::Code::GOOD;
                polledData.AddressType = jvs::adtp_e::NUMERIC;
                polledData.Address.Numeric = address;
                polledData.Timestamp = timestampInMillis;

                /**
                 * @todo ModbusTCP 클래스에서 얻은 상태 코드에 따라서 MUFFIN 상태 코드로 변환하는 작업이 필요합니다.
                 *       현재는 시간 상 IsOK로 Boolean 값을 받지만 향후에는 MUFFIN Status::Code로 변환해야 합니다.
                 */
                switch (area)
                {
                case jvs::node_area_e::COILS:
                    datum = mPolledDataTable.RetrieveCoil(slaveID, address);
                    goto BIT_MEMORY;
                case jvs::node_area_e::DISCRETE_INPUT:
                    datum = mPolledDataTable.RetrieveDiscreteInput(slaveID, address);
                BIT_MEMORY:
                    if (datum.IsOK == false)
                    {
                        LOG_ERROR(logger,"TCP POLLING ERROR");
                        polledData.StatusCode = Status::Code::BAD;
                        polledData.Value.Boolean = datum.Value == 1 ? true : false;
                    }
                    else
                    {
                        polledData.StatusCode = Status::Code::GOOD;
                        polledData.Value.Boolean = datum.Value == 1 ? true : false;
                    }
                    polledData.ValueType = jvs::dt_e::BOOLEAN;
                    vectorPolledData.emplace_back(polledData);
                    node->VariableNode.Update(vectorPolledData);
                    break;
                case jvs::node_area_e::INPUT_REGISTER:
                    vectorPolledData.reserve(quantity);
                    for (size_t i = 0; i < quantity; ++i)
                    {
                        datum = mPolledDataTable.RetrieveInputRegister(slaveID, address + i);
                        polledData.StatusCode = datum.IsOK ? Status::Code::GOOD : Status::Code::BAD;
                        polledData.ValueType = jvs::dt_e::UINT16;
                        polledData.Value.UInt16 = datum.Value;
                        vectorPolledData.emplace_back(polledData);
                    }
                    goto REGISTER_MEMORY;
                case jvs::node_area_e::HOLDING_REGISTER:
                    vectorPolledData.reserve(quantity);
                    for (size_t i = 0; i < quantity; ++i)
                    {
                        datum = mPolledDataTable.RetrieveHoldingRegister(slaveID, address + i);
                        polledData.StatusCode = datum.IsOK ? Status::Code::GOOD : Status::Code::BAD;                     
                        polledData.ValueType = jvs::dt_e::UINT16;
                        polledData.Value.UInt16 = datum.Value;
                        vectorPolledData.emplace_back(polledData);
                    }
                REGISTER_MEMORY:
                    node->VariableNode.Update(vectorPolledData);
                    break;
                default:
                    break;
                }
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status ModbusTCP::pollCoil(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();

            mModbusTCPClient->requestFrom(slaveID, COILS, startAddress, pollQuantity);
            delay(mScanRate);
            // const char* lastError = mModbusTCPClient->lastError();
            // mModbusTCPClient->clearError();

            // if (lastError != nullptr)
            // {
            //     ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
            //     for (size_t i = 0; i < pollQuantity; i++)
            //     {
            //         const uint16_t address = startAddress + i;
            //         mPolledDataTable.UpdateCoil(slaveID, address, INVALID_VALUE);
            //     }
            //     continue;
            // }


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = mModbusTCPClient->read();
                
                switch (value)
                {
                case 1:
                case 0:
                    mPolledDataTable.UpdateCoil(slaveID, address, value);
                    continue;
                default:
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateCoil(slaveID, address, INVALID_VALUE);
                }
            }
        }

        return ret;
    }

    Status ModbusTCP::pollDiscreteInput(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            mModbusTCPClient->requestFrom(slaveID, DISCRETE_INPUTS, startAddress, pollQuantity);
            delay(mScanRate);
            // const char* lastError = mModbusTCPClient->lastError();
            // mModbusTCPClient->clearError();

            // if (lastError != nullptr)
            // {
            //     LOG_ERROR(logger, "FAILED TO POLL: %s", lastError);
            //     ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
            //     for (size_t i = 0; i < pollQuantity; i++)
            //     {
            //         const uint16_t address = startAddress + i;
            //         mPolledDataTable.UpdateDiscreteInput(slaveID, address, INVALID_VALUE);
            //     }
            //     continue;
            // }


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = mModbusTCPClient->read();
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

    Status ModbusTCP::pollInputRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            mModbusTCPClient->requestFrom(slaveID, INPUT_REGISTERS, startAddress, pollQuantity);
            delay(mScanRate);
            // const char* lastError = mModbusTCPClient->lastError();
            // mModbusTCPClient->clearError();

            // if (lastError != nullptr)
            // {
            //     LOG_ERROR(logger, "[INPUT REGISTERS] FAILED TO POLL: %s", lastError);
            //     ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
            //     for (size_t i = 0; i < pollQuantity; i++)
            //     {
            //         const uint16_t address = startAddress + i;
            //         mPolledDataTable.UpdateInputRegister(slaveID, address, INVALID_VALUE);
            //     }
            //     continue;
            // }


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int32_t value = mModbusTCPClient->read();
                
                if (value == -1)
                {
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateInputRegister(slaveID, address, INVALID_VALUE);
                    // return ret;
                }
                else
                {
                    mPolledDataTable.UpdateInputRegister(slaveID, address, (uint16_t)value);
                }
            }
        }

        return ret;
    }

    Status ModbusTCP::pollHoldingRegister(const uint8_t slaveID, const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;

        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();
            mModbusTCPClient->requestFrom(slaveID, HOLDING_REGISTERS, startAddress, pollQuantity);
            delay(mScanRate);
            // const char* lastError = mModbusTCPClient->lastError();
            // mModbusTCPClient->clearError();

            // if (lastError != nullptr)
            // {
            //     LOG_ERROR(logger, "[HOLDING REGISTERS] FAILED TO POLL: %s, startAddress : %u, pollQuantity : %u", lastError, startAddress, pollQuantity);
            //     ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
            //     for (size_t i = 0; i < pollQuantity; i++)
            //     {
            //         const uint16_t address = startAddress + i;
            //         mPolledDataTable.UpdateHoldingRegister(slaveID, address, INVALID_VALUE);
            //     }
            //     continue;
            // }


            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int32_t value = mModbusTCPClient->read();
                
                // LOG_WARNING(logger, "[HOLDING REGISTERS][Address: %u] value : %d", address, value);
                if (value == -1)
                {
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateHoldingRegister(slaveID, address, INVALID_VALUE);
                    // return ret;
                }
                else
                {
                    mPolledDataTable.UpdateHoldingRegister(slaveID, address, value);
                }
            }
        }

        return ret;
    }

    modbus::datum_t ModbusTCP::GetAddressValue(const uint8_t slaveID, const uint16_t address, const jvs::node_area_e area)
    {
        modbus::datum_t data;
        data.IsOK = false;
        switch (area)
        {
        case jvs::node_area_e::COILS :
            data = mPolledDataTable.RetrieveCoil(slaveID,address);
            break;
        case jvs::node_area_e::DISCRETE_INPUT :
            data = mPolledDataTable.RetrieveDiscreteInput(slaveID,address);
            break;
        case jvs::node_area_e::INPUT_REGISTER :
            data = mPolledDataTable.RetrieveInputRegister(slaveID,address);
            break;
        case jvs::node_area_e::HOLDING_REGISTER :
            data = mPolledDataTable.RetrieveHoldingRegister(slaveID,address);
            break;
        default:
            LOG_DEBUG(logger,"ADDRESS HAS NO DATA!!");
            break;
        }
       
        return data;
    }

    IPAddress ModbusTCP::GetServerIP()
    {
        return mServerIP;
    }

    uint16_t ModbusTCP::GetServerPort()
    {
        return mServerPort;
    }

    void ModbusTCP::SetTimeoutError()
    {
        const auto RetrieveEntireNode = mNodeTable.RetrieveEntireNode();
        if (RetrieveEntireNode.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SLAVE ID FOR POLLING: %s", RetrieveEntireNode.first.c_str());
            return;
        }

        for (const auto& node : RetrieveEntireNode.second)
        {
            node->VariableNode.UpdateError();
        }
    }
#if defined(MT11)
    void ModbusTCP::SetModbusTCPClient(ModbusTCPClient* modbusTcpClient, w5500::EthernetClient* ethClient)
    {
        mModbusTCPClient = modbusTcpClient;
        mClient = ethClient;
    }
#endif
}