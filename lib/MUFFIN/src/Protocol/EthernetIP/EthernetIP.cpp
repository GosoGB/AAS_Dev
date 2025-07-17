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
        mScanRate   = config->GetScanRate().second;

        mAddressTable.DebugPrint();
        mAddressArrayTable.DebugPrint();
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
        
        // LOG_DEBUG(logger,"Session opened");

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
                LOG_DEBUG(logger,"nodeArrayIndex SIZE  : %d ",nodeArrayIndex.size());
                uint16_t startIndex = nodeArrayIndex[0][0];
                uint16_t count = nodeArrayIndex[0][1];
                LOG_INFO(logger,"startIndex : %d || count : %d",startIndex,count);
                ret = mAddressArrayTable.Update(reference->VariableNode.GetAddress().String, startIndex, count);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO UPDATE ADDRESS TABLE: %s", ret.c_str());
                    return Status(Status::Code::BAD);
                }
            }
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
        size_t ArrayBatchCount = mAddressArrayTable.GetArrayBatchCount();

        if (ArrayBatchCount != 0)
        {
            std::vector<tag_array_entry_t> tagArrayEntry = mAddressArrayTable.RetrieveTable(); 
            for(auto& entry : tagArrayEntry)
            {
                std::vector<cip_data_t> readValues;
                delay(mScanRate);
                if (readTagExt(mEipSession, entry.tagName, entry.startIndex, entry.count, readValues))
                {  
                    // LOG_DEBUG(logger,"[GOOD] TAG : %s, START INDEX : %d, COUNT : %d", entry.tagName.c_str(), entry.startIndex, entry.count);
                    Status ret = mPolledArrayDataTable.UpdateArrayRange(entry.tagName, entry.startIndex, readValues);   
                    if (ret != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger,"FAIL TO UPDATE POLLED ARRAY TABLE");
                        continue;
                    }
                    
                }   
                else
                {   
                    LOG_ERROR(logger, "POLLING ERROR FOR TAG ARRAY READ");
                    LOG_ERROR(logger,"[BAD] TAG : %s, START INDEX : %d, COUNT : %d", entry.tagName.c_str(), entry.startIndex, entry.count);
                    for (size_t i = 0; i < entry.count; i++)
                    {
                        cip_data_t datum;
                        datum.Code = 0xff; // 임시 에러 코드
                        readValues.emplace_back(datum);
                    }

                    Status ret = mPolledArrayDataTable.UpdateArrayRange(entry.tagName, entry.startIndex, readValues);   
                    if (ret != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger,"FAIL TO UPDATE POLLED ARRAY TABLE");
                        continue;
                    }
                }
            }
        }
        
        if (batchCount != 0)
        {
            for (size_t i = 0; i < batchCount; i++)
            {
                std::vector<std::string> retrievedTagInfo = mAddressTable.RetrieveTagsByBatch(i);
                std::vector<cip_data_t> readValues;
                delay(mScanRate);
                if (readTagsMSR(mEipSession, retrievedTagInfo, readValues))
                {
                    for (size_t i = 0; i < retrievedTagInfo.size(); i++)
                    {
                        const auto& tagName = retrievedTagInfo.at(i);
                        const auto& result = readValues.at(i);

                        try
                        {
                            auto it = mPolledDataTable.find(tagName);
                            if (it == mPolledDataTable.end())
                            {
                                mPolledDataTable.emplace(tagName, result);
                            }
                            else
                            {
                                it->second = result;
                                
                            }
                        }
                        catch (const std::bad_alloc& e)
                        {
                            LOG_ERROR(logger, "OUT OF MEMORY: %s - TAG: %s", e.what(), tagName.c_str());
                            // xSemaphoreGive(xSemaphoreEthernetIP);
                            return Status(Status::Code::BAD_OUT_OF_MEMORY);
                        }
                        catch (const std::exception& e)
                        {
                            LOG_ERROR(logger, "EXCEPTION: %s - TAG: %s", e.what(), tagName.c_str());
                            // xSemaphoreGive(xSemaphoreEthernetIP);
                            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
                        }
                    }
                }
                else
                {   
                    LOG_ERROR(logger, "POLLING ERROR FOR TAG");
                    for (size_t i = 0; i < retrievedTagInfo.size(); i++)
                    {
                        std::string tagName = retrievedTagInfo.at(i);
                        LOG_ERROR(logger,"[BAD] TAG : %s", tagName.c_str());
                        cip_data_t datum;
                        datum.Code = 0xff; // 임시 에러 코드

                        try
                        {
                            auto it = mPolledDataTable.find(tagName);
                            if (it == mPolledDataTable.end())
                            {
                                mPolledDataTable.emplace(tagName, datum);
                            }
                            else
                            {
                                it->second = datum;
                                
                            }
                        }
                        catch (const std::bad_alloc& e)
                        {
                            LOG_ERROR(logger, "OUT OF MEMORY: %s - TAG: %s", e.what(), tagName.c_str());
                            // xSemaphoreGive(xSemaphoreEthernetIP);
                            return Status(Status::Code::BAD_OUT_OF_MEMORY);
                        }
                        catch (const std::exception& e)
                        {
                            LOG_ERROR(logger, "EXCEPTION: %s - TAG: %s", e.what(), tagName.c_str());
                            // xSemaphoreGive(xSemaphoreEthernetIP);
                            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
                        }
                    }
                    
                }
            }  
        }

        // xSemaphoreGive(xSemaphoreEthernetIP);
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

            std::vector<cip_data_t> vPolledData;

            if (arrayIndex.size() == 0)
            {
                cip_data_t datum;

                auto it = mPolledDataTable.find(address);
                if (it != mPolledDataTable.end()) 
                {
                    datum = it->second;
                } 
                else 
                {
                    LOG_ERROR(logger,"[%s] TAG IS NOT EXIST",address.c_str());
                    return Status(Status::Code::BAD);
                }
            
                // 단일 값 처리
                std::vector<im::poll_data_t> vectorPolledData;
                vectorPolledData.reserve(datum.RawData.size());
                im::poll_data_t polledData;
                /**
                 * @todo EthernetIP 상태코드가 현재 제대로 구현이 안되어있음 확인 후 추가 개발이 필요함 @김주성 
                 * 
                 */
                
                 // 임시 상태코드 에러
                if (datum.Code == 0xff)
                {
                    polledData.StatusCode = Status::Code::BAD;
                }
                else
                {
                    polledData.StatusCode = Status::Code::GOOD;
                }
                
                
                polledData.AddressType = jvs::adtp_e::STRING;
                strcpy(polledData.Address.String, address.c_str());
                polledData.Timestamp = datum.Timestamp;
                ret = cipDataConvertToPollData(datum, &polledData);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO CONVERT CIP TYPE TO POLLED DATA TYPE: %s", ret.c_str());
                    return ret;
                }

                for (size_t i = datum.RawData.size(); i-- > 0; )
                {
                    polledData.ValueType = jvs::dt_e::UINT8;
                    polledData.Value.UInt8 = datum.RawData.at(i);
                    vectorPolledData.emplace_back(polledData);
                }
                
                node->VariableNode.Update(vectorPolledData);
            }
            else
            {
                uint16_t startIndex = arrayIndex[0][0];
                uint16_t count = arrayIndex[0][1];
                
                ret = mPolledArrayDataTable.GetArrayRange(address, startIndex, count, vPolledData);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO GET ARRAY RANGE: TAG='%s', START_INDEX=%zu, COUNT=%zu, STATUS=%s",
                        address.c_str(),
                        startIndex,
                        count,
                        ret.c_str()
                    );
                    return ret;
                }

                std::vector<im::poll_data_t> vectorPolledData;
                vectorPolledData.reserve(count);
                for (auto& datum : vPolledData)
                {
                    im::poll_data_t polledData;
                    /**
                     * @todo EthernetIP 상태코드가 현재 제대로 구현이 안되어있음 확인 후 추가 개발이 필요함 @김주성 
                     * 
                     */
                    
                    polledData.StatusCode = Status::Code::GOOD;
                    polledData.AddressType = jvs::adtp_e::STRING;
                    strcpy(polledData.Address.String, address.c_str());
                    polledData.Timestamp = vPolledData.at(0).Timestamp;
                    ret = cipDataConvertToPollData(datum, &polledData);
                    if (ret != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger, "FAILED TO CONVERT CIP TYPE TO POLLED DATA TYPE: %s", ret.c_str());
                        return ret;
                    }
                    vectorPolledData.emplace_back(polledData);
                }
                node->VariableNode.Update(vectorPolledData);
            }
        }
        
        return Status(Status::Code::GOOD);
    }

    cip_data_t EthernetIP::GetSingleAddressValue(std::string tag)
    {
        cip_data_t datum;
        
        auto it = mPolledDataTable.find(tag);
        if (it != mPolledDataTable.end()) 
        {
            datum = it->second;
        } 
        else 
        {
            //임시 에러 코드
            datum.Code = 0xff; 
            LOG_ERROR(logger,"[%s] TAG IS NOT EXIST",tag.c_str());
        }

        return datum;
    }

    bool EthernetIP::isPossibleToConvert(std::string& data)
    {
        bool decimalFound = false;
        size_t start = (data[0] == '-') ? 1 : 0;
        for (size_t i = start; i < data.length(); ++i) 
        {
            char c = data[i];
            if (c == '.') 
            {
                if (decimalFound) 
                {
                    return false;
                }
                decimalFound = true;
            } 
            else if (!isdigit(c)) 
            {
                return false;
            }
        }

        return true;
    }

    Status EthernetIP::StringConvertToCipData(std::string& data, cip_data_t* output)
    {
        switch (output->DataType)
        {
        case CipDataType::BOOL:
        {
            bool strData = data == "0" ? 0 : 1;
            output->RawData.emplace_back(static_cast<uint8_t>(strData));
            break;
        }
        case CipDataType::SINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            int8_t strData = static_cast<int8_t>(std::stoi(data));
            output->RawData.emplace_back(static_cast<uint8_t>(strData));
            break;
        }
        case CipDataType::USINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint8_t strData = static_cast<uint8_t>(std::stoul(data));
            output->RawData.emplace_back(strData);
            break;
        }
        case CipDataType::BYTE:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint8_t strData = static_cast<uint8_t>(std::stoul(data));
            output->RawData.emplace_back(strData);
            break;
        }
        case CipDataType::INT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            int16_t strData = static_cast<int16_t>(std::stoi(data));
            output->RawData.emplace_back(static_cast<uint8_t>(strData & 0xFF));
            output->RawData.emplace_back(static_cast<uint8_t>((strData >> 8) & 0xFF));
            break;
        }
        case CipDataType::UINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint16_t strData = static_cast<uint16_t>(std::stoul(data));
            output->RawData.emplace_back(static_cast<uint8_t>(strData & 0xFF));
            output->RawData.emplace_back(static_cast<uint8_t>((strData >> 8) & 0xFF));
            break;
        }
        case CipDataType::WORD:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint16_t strData = static_cast<uint16_t>(std::stoul(data));
            output->RawData.emplace_back(static_cast<uint8_t>(strData & 0xFF));
            output->RawData.emplace_back(static_cast<uint8_t>((strData >> 8) & 0xFF));
            break;
        }
        case CipDataType::DINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            int32_t strData = static_cast<int32_t>(std::stoi(data));
            for (int i = 0; i < 4; ++i)
                output->RawData.emplace_back(static_cast<uint8_t>((strData >> (8 * i)) & 0xFF));
            break;
        }
        case CipDataType::UDINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint32_t strData = static_cast<uint32_t>(std::stoul(data));
            for (int i = 0; i < 4; ++i)
                output->RawData.emplace_back(static_cast<uint8_t>((strData >> (8 * i)) & 0xFF));
            break;
        }
        case CipDataType::DWORD:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint32_t strData = static_cast<uint32_t>(std::stoul(data));
            for (int i = 0; i < 4; ++i)
                output->RawData.emplace_back(static_cast<uint8_t>((strData >> (8 * i)) & 0xFF));
            break;
        }
        case CipDataType::REAL:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            float strData = std::stof(data);
            uint8_t bytes[4];
            std::memcpy(bytes, &strData, sizeof(float));
            for (int i = 0; i < 4; ++i)
                output->RawData.emplace_back(bytes[i]);
            break;
        }
        case CipDataType::LREAL:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            double strData = std::stod(data);
            uint8_t bytes[8];
            std::memcpy(bytes, &strData, sizeof(double));
            for (int i = 0; i < 8; ++i)
                output->RawData.emplace_back(bytes[i]);
            break;
        }
        case CipDataType::LINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            int64_t strData = static_cast<int64_t>(std::stoll(data));
            for (int i = 0; i < 8; ++i)
                output->RawData.emplace_back(static_cast<uint8_t>((strData >> (8 * i)) & 0xFF));
            break;
        }
        case CipDataType::ULINT:
        {
            if (!isPossibleToConvert(data))
                return Status(Status::Code::BAD_TYPE_MISMATCH);

            uint64_t strData = static_cast<uint64_t>(std::stoull(data));
            for (int i = 0; i < 8; ++i)
                output->RawData.emplace_back(static_cast<uint8_t>((strData >> (8 * i)) & 0xFF));
            break;
        }
        case CipDataType::STRING:
        {
            for (char ch : data)
                output->RawData.emplace_back(static_cast<uint8_t>(ch));
            break;
        }
        default:
            LOG_ERROR(logger, "NOT SUPPORTED FOR THIS DATA TYPE");
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }

        return Status(Status::Code::GOOD);
    }

    Status EthernetIP::cipDataConvertToPollData(cip_data_t& data, im::poll_data_t* output)
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
        case CipDataType::USINT:
            output->ValueType = jvs::dt_e::UINT8;
            output->Value.UInt8 = data.Value.USINT;
            break;
        case CipDataType::INT:
            output->ValueType = jvs::dt_e::INT16;
            output->Value.Int16 = data.Value.INT;
            break;
        case CipDataType::BYTE:
            output->ValueType = jvs::dt_e::UINT8;
            output->Value.UInt8 = data.Value.BYTE;
            break;
        case CipDataType::WORD:
            output->ValueType = jvs::dt_e::UINT16;
            output->Value.UInt16 = data.Value.WORD;
            break;
        case CipDataType::UINT:
            output->ValueType = jvs::dt_e::UINT16;
            output->Value.UInt16 = data.Value.UINT;
            break;
        case CipDataType::DINT:
            output->ValueType = jvs::dt_e::INT32;
            output->Value.Int32 = data.Value.DINT;
            break;
        case CipDataType::DWORD:
            output->ValueType = jvs::dt_e::UINT32;
            output->Value.UInt32 = data.Value.DWORD;
            break;
        case CipDataType::UDINT:
            output->ValueType = jvs::dt_e::UINT32;
            output->Value.UInt32 = data.Value.UDINT;
            break;
        case CipDataType::REAL:
            output->ValueType = jvs::dt_e::FLOAT32;
            output->Value.Float32 = data.Value.REAL;
            break;
        case CipDataType::LINT:
            output->ValueType = jvs::dt_e::INT64;
            output->Value.Int64 = data.Value.LINT;
            break;
        case CipDataType::ULINT:
            output->ValueType = jvs::dt_e::UINT64;
            output->Value.UInt64 = data.Value.ULINT;
            break;
        case CipDataType::LREAL:
            output->ValueType = jvs::dt_e::FLOAT64;
            output->Value.Float64 = data.Value.LREAL;
            break;
        case CipDataType::STRING:
            output->ValueType = jvs::dt_e::STRING;
            strcpy(output->Value.String.Data, data.Value.STRING.Data);
            output->Value.String.Length = data.Value.STRING.Length;
            break;
        default:
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }

        return Status(Status::Code::GOOD);
    }



}}

#endif