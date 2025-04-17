/**
 * @file Melsec.cpp
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec 프로토콜 클래스를 선언합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */



#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "IM/Node/NodeStore.h"
#include "Melsec.h"
#include "MelsecMutex.h"
#include "IM/Node/Include/Utility.h"


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
        mMelsecClient.SetDataFormat(mDataformat);
        return (mMelsecClient.Begin(mServerIP.toString().c_str(), mServerPort, mPlcSeries));
    }

    Status Melsec::Config(jvs::config::Melsec* config)
    {
        /**
         * @todo SlaveID가 없기 때문에 기존 테이블을 이용하려고 임시로 1로 고정시켜 두었음
         *       Melsec을 위한 노드테이블을 다시 만들것인지 이야기 해야함 @김주성
         * 
         * @lsj  별도로 테이블을 만들 필요는 없을 것 같은데... 
         *       다만 1이라는 매직 넘버를 쓰는 건 안 좋아요
         *       대신 constexpr uint8_t DEFAULT_SLAVE_NUMBER 
         *       같은 상수를 하나 정의해서 쓰는 게 좋습니다.
         * @lsj  NodeReference 추가에 실패하는 경우를 처리하도록 수정 필요함
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

            // @lsj node_area_e 보다 직관적인 거 있으면 좋겠는데...
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

    // @lsj 이거 모드버스랑 겹치는 부분이 있는 거 같은데 하나로 합치는 게 나을 수도 있겠다 싶네요
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
                if (im::IsBitArea(area))
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

                if (im::IsBitArea(area))
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

            uint16_t response[pollQuantity+1];  // @lsj 메모리 할당과 초기화는 항상 같이 하는 게 좋아요
            int result = mMelsecClient.ReadBits(area,startAddress,pollQuantity,response);
            delay(80); // @lsj 딜레이 없이도 데이터 수집에 무리가 없는지 확인 부탁드려요

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
            int result = mMelsecClient.ReadWords(area,startAddress,pollQuantity,response);
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

        if (im::IsBitArea(area))
        {
            data = mPolledDataTable.RetrieveBitArea(slaveID, address, area);
        }
        else
        {
            data = mPolledDataTable.RetrieveWordArea(slaveID, address, area);
        }
        
        return data;
        
    }
}
