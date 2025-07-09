/**
 * @file EthernetIP.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MFM으로 설정한 Ethernet/IP 프로토콜에 따라 데이터를 수집, 처리, 관리하는 클래스를 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "IM/Node/NodeStore.h"
#include "IM/Node/Include/Utility.h"
#include "EthernetIP.h"
#include "EthernetIpMutex.h"




namespace muffin { namespace ethernetIP {


    EthernetIP::EthernetIP(EIPSession EipSession)
    :mEipSession(EipSession)
    {

    }

    EthernetIP::~EthernetIP()
    {

    }

    Status EthernetIP::Config(jvs::config::EthernetIP *config)
    {
        addNodeReferences(config->GetNodes().second);
        mServerIP   = config->GetIPv4().second;
        mServerPort = config->GetPort().second;

        return Status(Status::Code::GOOD); 
    }

    void EthernetIP::Clear()
    {
        mNodeTable.Clear();
        mAddressTable.Clear();
    }

    bool EthernetIP::Connect()
    {
        if (!eipInit(mEipSession, mServerIP, mServerPort)) 
        {
            LOG_ERROR(logger,"FAILED TO ETHERNETIP INIT");
            return false;
        }

        if (!registerSession(mEipSession)) 
        {
            LOG_ERROR(logger,"FAILED TO REGISTER SESSION");
            return false;
        }
        
        LOG_DEBUG(logger,"Session opened");

        return true;
    }

    Status EthernetIP::addNodeReferences(const std::vector<std::__cxx11::string>& vectorNodeID)
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
            ret = mNodeTable.Update(*reference);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE NODE RERENCE TABLE: %s", nodeID.c_str());
                return ret;
            }

            std::vector<std::array<uint16_t, 2>> nodeArrayIndex = reference->VariableNode.GetArrayIndex();
            LOG_DEBUG(logger,"nodeArrayIndex size : %d", nodeArrayIndex.size());
            
            if (nodeArrayIndex.size() == 0)
            {
                ret = mAddressTable.Update(reference->VariableNode.GetAddress().String);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS TABLE: %s", ret.c_str());
                    return Status(Status::Code::BAD);
                }
            }
            else
            {
                /**
                 * @todo 
                 * v1.5.0에서는 1차원 배열 기준으로 슬라이싱이 가능하며 batch table 을 구성하고 있음
                 * 추후 다차원 배열에 대해 read 명령을 처리할 수 있는 table을 구현하도록 수정해야함 @김주성
                 * 
                 */

            }
            mAddressTable.DebugPrint();
        }

        return Status(Status::Code::GOOD);
    }

    void EthernetIP::SetTimeoutError()
    {

    }

    Status EthernetIP::Poll()
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

    Status EthernetIP::implementPolling()
    {
        size_t batchCount = mAddressTable.GetBatchCount();

        if (xSemaphoreTake(xSemaphoreEthernetIP, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[EthernetIP] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }


        for (size_t i = 0; i < batchCount; i++)
        {
            std::vector<std::string> retrievedTagInfo = mAddressTable.RetrieveTagsByBatch(i);
            
            std::vector<cip_data_t> readValues;
            if (readTagsMSR(mEipSession, retrievedTagInfo, readValues))
            {
                for (size_t i = 0; i < readValues.size(); i++)
                {
                    const auto& tagName = retrievedTagInfo.at(i);
                    const auto& result = readValues.at(i);

                    LOG_DEBUG(logger,"tagName : %s",tagName.c_str());
                    printCipData(result);

                    try
                    {
                        auto it = mPolledDataTable.find(tagName);
                        if (it == mPolledDataTable.end())
                        {
                            // 태그 없음 → 벡터 하나 만들어 넣기
                            mPolledDataTable.emplace(tagName, std::vector<cip_data_t>{result});
                        }
                        else
                        {
                            // 현재는 하나의 값만 들어오고 처리하기 때문에 이렇게 만듬
                            it->second.clear();
                            it->second.emplace_back(result);
                            
                        }
                    }
                    catch (const std::bad_alloc& e)
                    {
                        LOG_ERROR(logger, "OUT OF MEMORY: %s - TAG: %s", e.what(), tagName.c_str());
                        xSemaphoreGive(xSemaphoreEthernetIP);
                        return Status(Status::Code::BAD_OUT_OF_MEMORY);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(logger, "EXCEPTION: %s - TAG: %s", e.what(), tagName.c_str());
                        xSemaphoreGive(xSemaphoreEthernetIP);
                        return Status(Status::Code::BAD_UNEXPECTED_ERROR);
                    }
                }
            }
            else
            {   
                LOG_ERROR(logger, "POLLING ERROR FOR TAG");
                xSemaphoreGive(xSemaphoreEthernetIP);
                return Status(Status::Code::BAD);
            }

        }  
        xSemaphoreGive(xSemaphoreEthernetIP);
        return Status(Status::Code::GOOD);
    }

    Status EthernetIP::updateVariableNodes()
    {
        Status ret(Status::Code::UNCERTAIN);
        
        const auto retrievedNodeInfo = mNodeTable.RetrieveEntireNode();
        if (retrievedNodeInfo.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE NODE REFERENCES: %s", retrievedNodeInfo.first.c_str());
            return Status(Status::Code::BAD_NOT_FOUND);
        }

        for (auto& node : retrievedNodeInfo.second)
        {
            std::vector<std::array<uint16_t, 2>> arrayIndex = node->VariableNode.GetArrayIndex();
            std::string address = node->VariableNode.GetAddress().String;
            const uint16_t quantity = node->VariableNode.GetQuantity();

            std::vector<cip_data_t> vPolledData;
            LOG_DEBUG(logger,"mPolledDataTable SIZE : %d" , mPolledDataTable.size());
            auto it = mPolledDataTable.find(address);
            if (it != mPolledDataTable.end()) 
            {
                vPolledData = it->second;
            } 
            else 
            {
                LOG_ERROR(logger,"[%s] TAG IS NOT EXIST",address.c_str());
                return Status(Status::Code::BAD);
            }
            

            if (arrayIndex.size() == 0)
            {
                cip_data_t datum = vPolledData.at(0);
                // 단일 값 처리
                std::vector<im::poll_data_t> vectorPolledData;
                vectorPolledData.reserve(quantity);
                im::poll_data_t polledData;
                /**
                 * @todo EthernetIP 상태코드가 현재 제대로 구현이 안되어있음 확인 후 추가 개발이 필요함 @김주성 
                 * 
                 */
                
                polledData.StatusCode = Status::Code::GOOD;
                polledData.AddressType = jvs::adtp_e::STRING;
                strcpy(polledData.Address.String, address.c_str());
                polledData.Timestamp = datum.Timestamp;
                LOG_INFO(logger,"TAG : %s || DATA TYPE : %d || DATA : %d", address.c_str() , datum.DataType, datum.Value.SINT );
                LOG_INFO(logger,"RAW DATA SIZE : %d",datum.RawData.size());
                ret = convertToValue(datum, &polledData);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO CONVERT CIP TYPE TO POLLED DATA TYPE: %s", ret.c_str());
                    return ret;
                }


                for (size_t i = 0; i < datum.RawData.size(); i++)
                {
                    polledData.ValueType = jvs::dt_e::UINT8;
                    polledData.Value.UInt8 = datum.RawData.at(i);
                    vectorPolledData.emplace_back(polledData);
                }
                
                node->VariableNode.Update(vectorPolledData);

            }
        }
        
        return Status(Status::Code::GOOD);
    }

    Status EthernetIP::convertToValue(cip_data_t& data, im::poll_data_t* output)
    {
        switch (data.DataType)
        {
        case CipDataType::BOOL:
            output->ValueType = jvs::dt_e::BOOLEAN;
            output->Value.Boolean = data.Value.BOOL;
            break;
        case CipDataType::SINT:
            output->ValueType = jvs::dt_e::INT8;
            output->Value.Int8 = data.Value.SINT;
            break;
        case CipDataType::INT:
            output->ValueType = jvs::dt_e::INT16;
            output->Value.Int16 = data.Value.INT;
            break;
        case CipDataType::DINT:
            output->ValueType = jvs::dt_e::INT32;
            output->Value.Int32 = data.Value.DINT;
            break;
        case CipDataType::REAL:
            output->ValueType = jvs::dt_e::FLOAT32;
            output->Value.Float32 = data.Value.REAL;
            break;
        case CipDataType::LINT:
            output->ValueType = jvs::dt_e::INT64;
            output->Value.Int64 = data.Value.LINT;
            break;
        case CipDataType::STRING:
            output->ValueType = jvs::dt_e::STRING;
            output->Value.String = data.Value.STRING;
            return Status(Status::Code::GOOD);
        default:
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }

        return Status(Status::Code::GOOD);
    }



}}

#endif