/**
 * @file Melsec.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "IM/Node/NodeStore.h"
#include "Melsec.h"
#include "MelsecMutex.h"


namespace muffin {

    Melsec::Melsec()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Melsec::~Melsec()
    {   
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    bool Melsec::Connect()
    {
        mMelsecClient.setDataFormat(static_cast<MCDataFormat>(mDataformat));

        return (mMelsecClient.begin(mServerIP.toString().c_str(),mServerPort,static_cast<MitsuPLCSeries>(mPlcSeries)));
    }

    Status Melsec::Config(jvs::config::Melsec* config)
    {
        /**
         * @todo SlaveID가 없기 때문에 기존 테이블을 이용하려고 임시로 1로 고정시켜 두었음
         *       Melsec을 위한 노드테이블을 다시 만들것인지 이야기 해야함 @김주성
         * 
         */
        addNodeReferences(1, config->GetNodes().second);
        mServerIP   = config->GetIPv4().second;
        mServerPort = config->GetPort().second;
        mPlcSeries  = config->GetPlcSeies().second;
        mDataformat = config->GetDataFormat().second;
        return Status(Status::Code::GOOD);
    }

    void Melsec::Clear()
    {
        mNodeTable.Clear();
        mAddressTable.Clear();
        mPolledDataTable.Clear();
    }

    Status Melsec::addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID)
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

    im::NumericAddressRange Melsec::createAddressRange(const uint16_t address, const uint16_t quantity) const
    {
        return AddressRange(address, quantity);
    }

    IPAddress Melsec::GetServerIP()
    {
        return mServerIP;
    }

    uint16_t Melsec::GetServerPort()
    {
        return mServerPort;
    }

    Status Melsec::Poll()
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

    Status Melsec::implementPolling()
    {
        Status ret(Status::Code::UNCERTAIN);

        const auto retrievedSlaveInfo = mAddressTable.RetrieveEntireSlaveID();
        if (retrievedSlaveInfo.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SLAVE ID FOR POLLING: %s", retrievedSlaveInfo.first.c_str());
            return Status(Status::Code::BAD);
        }

        if (xSemaphoreTake(xSemaphoreMelsec, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[MELSEC] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
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
                LOG_ERROR(logger, "FAILED TO RETRIEVE MELSEC AREA FOR POLLING: %s", retrievedAreaInfo.first.c_str());
                return Status(Status::Code::BAD);
            }

            for (const auto& area : retrievedAreaInfo.second)
            {
                const auto& addressSetToPoll = retrievedAddressInfo.second.RetrieveAddressRange(area);
                if (isBitArea(area))
                {
                    ret = bitsRead(area, addressSetToPoll);
                    continue;
                }
                else
                {
                    ret = wordsRead(area, addressSetToPoll);
                    continue;
                }
                
    
                if (ret != Status(Status::Code::GOOD))
                {
                    LOG_ERROR(logger, "FAILED TO POLL: %s, SlaveID : %u, AREA : %u", ret.c_str(), slaveID, static_cast<uint8_t>(area));
                }
            }
        }
        xSemaphoreGive(xSemaphoreMelsec);
        return ret;
    }

    Status Melsec::updateVariableNodes()
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

                if (isBitArea(area))
                {
                    datum = mPolledDataTable.RetrieveBitArea(slaveID, address, area);
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
                    polledData.ValueType = jvs::dt_e::BOOLEAN;
                    vectorPolledData.emplace_back(polledData);
                    node->VariableNode.Update(vectorPolledData);
                }
                else
                {
                    vectorPolledData.reserve(quantity);
                    for (size_t i = 0; i < quantity; ++i)
                    {
                        datum = mPolledDataTable.RetrieveWordArea(slaveID, address + i, area);
                        polledData.StatusCode = datum.IsOK ? Status::Code::GOOD : Status::Code::BAD;
                        polledData.ValueType = jvs::dt_e::UINT16;
                        polledData.Value.UInt16 = datum.Value;
                        vectorPolledData.emplace_back(polledData);
                    }
                    node->VariableNode.Update(vectorPolledData);
                }
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status Melsec::bitsRead(const jvs::node_area_e area,const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;
        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();

            bool response[pollQuantity+1];
            int result = mMelsecClient.readBits(ConvertToDeviceType(area),startAddress,pollQuantity,response);
            delay(80);

            if (result != pollQuantity) 
            {
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateBitArea(1, address, INVALID_VALUE, area);
                }
                continue;
            } 

            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int8_t value = response[i];
                switch (value)
                {
                case 1:
                case 0:
                    mPolledDataTable.UpdateBitArea(1, address, value, area);
                    continue;
                default:
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateBitArea(1, address, INVALID_VALUE, area);
                }
            }

        }
        return ret;

    }

    Status Melsec::wordsRead(const jvs::node_area_e area,const std::set<AddressRange>& addressRangeSet)
    {
        Status ret(Status::Code::GOOD);
        constexpr int8_t INVALID_VALUE = -1;
        for (const auto& addressRange : addressRangeSet)
        {
            const uint16_t startAddress = addressRange.GetStartAddress();
            const uint16_t pollQuantity = addressRange.GetQuantity();

            uint16_t response[pollQuantity];
            int result = mMelsecClient.readWords(ConvertToDeviceType(area),startAddress,pollQuantity,response);
            delay(80);

            if (result != pollQuantity) 
            {
                ret = Status(Status::Code::BAD_DATA_UNAVAILABLE);
                for (size_t i = 0; i < pollQuantity; i++)
                {
                    const uint16_t address = startAddress + i;
                    mPolledDataTable.UpdateWordArea(1, address, INVALID_VALUE, area);
                }
                continue;
            } 

            for (size_t i = 0; i < pollQuantity; i++)
            {
                const uint16_t address = startAddress + i;
                const int32_t value = response[i];
                if (value == -1)
                {
                    LOG_ERROR(logger, "DATA LOST: INVALID VALUE");
                    ret = Status::Code::BAD_DATA_LOST;
                    mPolledDataTable.UpdateWordArea(1, address, INVALID_VALUE, area);
                    return ret;
                }
                else
                {
                    mPolledDataTable.UpdateWordArea(1, address, value, area);
                }
            }

        }


        return ret;

    }

    modbus::datum_t Melsec::GetAddressValue(const uint8_t slaveID, const uint16_t address, const jvs::node_area_e area)
    {
        modbus::datum_t data;
        data.IsOK = false;

        if (isBitArea(area))
        {
            data = mPolledDataTable.RetrieveBitArea(slaveID, address, area);
        }
        else
        {
            data = mPolledDataTable.RetrieveWordArea(slaveID, address, area);
        }
        
        return data;
        
    }

    MitsuDeviceType Melsec::ConvertToDeviceType(const jvs::node_area_e area)
    {
        switch (area)
        {
        case jvs::node_area_e::SM:
            return MitsuDeviceType::SM;
        case jvs::node_area_e::X:
            return MitsuDeviceType::X;
        case jvs::node_area_e::Y:
            return MitsuDeviceType::Y;
        case jvs::node_area_e::M:
            return MitsuDeviceType::M;
        case jvs::node_area_e::L:
            return MitsuDeviceType::L;
        case jvs::node_area_e::F:
            return MitsuDeviceType::F;
        case jvs::node_area_e::V:
            return MitsuDeviceType::F;
        case jvs::node_area_e::B:
            return MitsuDeviceType::B;
        case jvs::node_area_e::TS:
            return MitsuDeviceType::TS;
        case jvs::node_area_e::TC:
            return MitsuDeviceType::TC;
        case jvs::node_area_e::LTS:
            return MitsuDeviceType::LTS;
        case jvs::node_area_e::LTC:
            return MitsuDeviceType::LTC;
        case jvs::node_area_e::STS:
            return MitsuDeviceType::STS;
        case jvs::node_area_e::STC:
            return MitsuDeviceType::STC;
        case jvs::node_area_e::LSTS:
            return MitsuDeviceType::LSTS;
        case jvs::node_area_e::LSTC:
            return MitsuDeviceType::LSTC;
        case jvs::node_area_e::CS:
            return MitsuDeviceType::CS;
        case jvs::node_area_e::CC:
            return MitsuDeviceType::CC;
        case jvs::node_area_e::LCS:
            return MitsuDeviceType::LCS;
        case jvs::node_area_e::LCC:
            return MitsuDeviceType::LCC;
        case jvs::node_area_e::SB:
            return MitsuDeviceType::SB;
        case jvs::node_area_e::S:
            return MitsuDeviceType::S;
        case jvs::node_area_e::DX:
            return MitsuDeviceType::DX;
        case jvs::node_area_e::DY:
            return MitsuDeviceType::DY;
        case jvs::node_area_e::SD:
            return MitsuDeviceType::SD;
        case jvs::node_area_e::D:
            return MitsuDeviceType::D;
        case jvs::node_area_e::W:
            return MitsuDeviceType::W;
        case jvs::node_area_e::TN:
            return MitsuDeviceType::TN;
        case jvs::node_area_e::CN:
            return MitsuDeviceType::CN;
        case jvs::node_area_e::SW:
            return MitsuDeviceType::SW;
        case jvs::node_area_e::Z:
            return MitsuDeviceType::Z;
        case jvs::node_area_e::LTN:
            return MitsuDeviceType::LTN;
        case jvs::node_area_e::STN:
            return MitsuDeviceType::STN;
        case jvs::node_area_e::LSTN:
            return MitsuDeviceType::LSTN;
        case jvs::node_area_e::LCN:
            return MitsuDeviceType::LCN;
        case jvs::node_area_e::LZ:
            return MitsuDeviceType::LZ;
        default:
            LOG_ERROR(logger,"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
            ASSERT((true),"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
            return MitsuDeviceType::D;
        }
    }

    bool Melsec::isBitArea(const jvs::node_area_e area)
    {
        switch (area)
        {
        case jvs::node_area_e::COILS:
        case jvs::node_area_e::DISCRETE_INPUT:
        case jvs::node_area_e::SM:
        case jvs::node_area_e::X:
        case jvs::node_area_e::Y:
        case jvs::node_area_e::M:
        case jvs::node_area_e::L:
        case jvs::node_area_e::F:
        case jvs::node_area_e::V:
        case jvs::node_area_e::B:
        case jvs::node_area_e::TS:
        case jvs::node_area_e::TC:
        case jvs::node_area_e::LTS:
        case jvs::node_area_e::LTC:
        case jvs::node_area_e::STS:
        case jvs::node_area_e::STC:
        case jvs::node_area_e::LSTS:
        case jvs::node_area_e::LSTC:
        case jvs::node_area_e::CS:
        case jvs::node_area_e::CC:
        case jvs::node_area_e::LCS:
        case jvs::node_area_e::LCC:
        case jvs::node_area_e::SB:
        case jvs::node_area_e::S:
        case jvs::node_area_e::DX:
        case jvs::node_area_e::DY:
            return true;
        case jvs::node_area_e::INPUT_REGISTER:
        case jvs::node_area_e::HOLDING_REGISTER:
        case jvs::node_area_e::SD:
        case jvs::node_area_e::D:
        case jvs::node_area_e::W:
        case jvs::node_area_e::TN:
        case jvs::node_area_e::CN:
        case jvs::node_area_e::SW:
        case jvs::node_area_e::Z:
            return false;
        /**
         * @todo DoubleWord 어떻게 처리할 것인지 확인 필요 @김주성 
         * 
         */
        case jvs::node_area_e::LTN:
        case jvs::node_area_e::STN:
        case jvs::node_area_e::LSTN:
        case jvs::node_area_e::LCN:
        case jvs::node_area_e::LZ:
            LOG_WARNING(logger,"Double Word")
            return false;
        default:
            LOG_ERROR(logger,"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
            ASSERT((true),"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
            return false;
        }
    }
}
