/**
 * @file Variable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 정의합니다.
 * 
 * @date 2024-11-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <cmath>
#include <iomanip>
#include <sstream>
#include <string.h>

#include "Common/Assert.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Logger/Logger.h"
#include "Protocol/MQTT/CDO.h"
#include "Variable.h"



namespace muffin { namespace im {

    Variable::Variable(const std::string& nodeID, const std::string& UID)
        : mModbusArea(false, jarvis::mb_area_e::COILS)
        , mBitIndex(false, 0)
        , mAddressQuantity(false, 1)
        , mNumericScale(false, jarvis::scl_e::NEGATIVE_1)
        , mNumericOffset(false, 0.0f)
        , mMapMappingRules(false, std::map<std::uint16_t, std::string>())
        , mVectorDataUnitOrders(false, std::vector<jarvis::DataUnitOrder>())
        , mFormatString(false, std::string())
        , mNodeID(nodeID)
        , mDeprecableUID(UID)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Variable::~Variable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void Variable::Init(const jarvis::config::Node* cin)
    {
        mAddressType              = cin->GetAddressType().second;
        mAddress                  = cin->GetAddrress().second;
        mVectorDataTypes          = cin->GetDataTypes().second;
        mHasAttributeEvent        = cin->GetAttributeEvent().second;
        mDeprecableDisplayName    = cin->GetDeprecableDisplayName().second;
        mDeprecableDisplayUnit    = cin->GetDeprecableDisplayUnit().second;                


        if (cin->GetModbusArea().first == Status::Code::GOOD)
        {
            mModbusArea.first   = true;
            mModbusArea.second  = cin->GetModbusArea().second;
        }
        
        if (cin->GetBitIndex().first == Status::Code::GOOD)
        {
            mBitIndex.first   = true;
            mBitIndex.second  = cin->GetBitIndex().second;
        }

        if (cin->GetNumericAddressQuantity().first == Status::Code::GOOD)
        {
            mAddressQuantity.first   = true;
            mAddressQuantity.second  = cin->GetNumericAddressQuantity().second;
        }
        
        if (cin->GetNumericScale().first == Status::Code::GOOD)
        {
            mNumericScale.first   = true;
            mNumericScale.second  = cin->GetNumericScale().second;
        }

        if (cin->GetNumericOffset().first == Status::Code::GOOD)
        {
            mNumericOffset.first   = true;
            mNumericOffset.second  = cin->GetNumericOffset().second;
        }
        
        if (cin->GetMappingRules().first == Status::Code::GOOD)
        {
            mMapMappingRules.first   = true;
            mMapMappingRules.second  = cin->GetMappingRules().second;
        }

        if (cin->GetDataUnitOrders().first == Status::Code::GOOD)
        {
            mVectorDataUnitOrders.first   = true;
            mVectorDataUnitOrders.second  = cin->GetDataUnitOrders().second;
        }
        
        if (cin->GetFormatString().first == Status::Code::GOOD)
        {
            mFormatString.first   = true;
            mFormatString.second  = cin->GetFormatString().second;
        }
    
        if (mMapMappingRules.first == true || mFormatString.first == true)
        {
            mDataType = jarvis::dt_e::STRING;
        }
        else
        {
            ASSERT((mVectorDataTypes.size() == 1), "DATA TYPE VECTOR SIZE MUST BE 1 WHEN FORMAT STRING IS DISABLED");
            mDataType = mVectorDataTypes[0];
        }
    }

    std::string createFormattedString(const std::string& format, std::vector<casted_data_t>& inputVector)
    {
        std::ostringstream oss;
        size_t formatSpecifierCount = 0;

        for (size_t i = 0; i < format.size(); ++i)
        {
            if (format[i] == '%' && (i + 1) < format.size())
            {
                char specifier = format[i + 1];
                if (specifier == '%')
                {
                    oss << '%';
                    ++i;
                    continue;
                }
                
                if ((formatSpecifierCount + 1) > inputVector.size())
                {
                    LOG_ERROR(logger, "NOT ENOUGH CASTED DATA FOR GIVEN FORMAT SPECIFIERS");
                    return "";
                }
                
                
                bool hasZeroPadding = false;
                bool hasLongLong    = false;
                int8_t width = 0;
                int8_t precision = -1;

                if (format[i + 1] == '0')
                {
                    hasZeroPadding = true;
                    ++i;
                }

                while ((i + 1) < format.size() && isdigit(format[i + 1]))
                {
                    width = width * 10 + (format[++i] - '0');
                }

                if ((i + 1) < format.size() && format[i + 1] == '.')
                {
                    ++i;
                    precision = 0;
                    while ((i + 1) < format.size() && isdigit(format[i + 1]))
                    {
                        precision = precision * 10 + (format[++i] - '0');
                    }
                }

                if ((i + 1) < format.size() && format[i + 1] == 'l')
                {
                    ++i;

                    if ((i + 1) < format.size() && format[i + 1] == 'l')
                    {
                        hasLongLong = true;
                        ++i;
                    }
                }

                const casted_data_t& castedData = inputVector[formatSpecifierCount++];
                switch (format[i + 1])
                {
                    case 'd':
                        if (hasLongLong == true)
                        {
                            switch (castedData.ValueType)
                            {
                                case jarvis::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jarvis::dt_e::INT64:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int64;
                                    break;
                                case jarvis::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT32:
                                {
                                    const int64_t value = castedData.Value.UInt32;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (castedData.ValueType)
                            {
                                case jarvis::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jarvis::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                        break;
                    
                    case 'u':
                        if (hasLongLong == true)
                        {
                            switch (castedData.ValueType)
                            {
                                case jarvis::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jarvis::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.UInt32;
                                    break;
                                case jarvis::dt_e::UINT64:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.UInt64;
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (castedData.ValueType)
                            {
                                case jarvis::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jarvis::dt_e::UINT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.UInt32;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    
                    case 'f':
                        if (castedData.ValueType == jarvis::dt_e::FLOAT32)
                        {
                            oss << std::fixed;
                            if (precision >= 0)
                            {
                                oss << std::setprecision(precision);
                            }
                            oss << std::setw(width) << castedData.Value.Float32;
                        }
                        else if (castedData.ValueType == jarvis::dt_e::FLOAT64)
                        {
                            oss << std::fixed;
                            if (precision >= 0)
                            {
                                oss << std::setprecision(precision);
                            }
                            oss << std::setw(width) << castedData.Value.Float64;
                        }
                        break;
                    
                    case 'c':
                    case 's':
                        if (castedData.ValueType == jarvis::dt_e::STRING)
                        {
                            oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.String.Data;
                        }
                        break;
                    
                    case 'x':
                    case 'X':
                        if (jarvis::dt_e::BOOLEAN < castedData.ValueType && castedData.ValueType < jarvis::dt_e::FLOAT32)
                        {
                            if (format[i + 1] == 'x')
                            {
                                switch (castedData.ValueType)
                                {
                                    case jarvis::dt_e::UINT8:
                                        oss << std::setw(width) << std::hex << std::setfill('0') << castedData.Value.UInt8;
                                        break;
                                    case jarvis::dt_e::UINT16:
                                        oss << std::setw(width) << std::hex << std::setfill('0') << castedData.Value.UInt16;
                                        break;
                                    case jarvis::dt_e::UINT32:
                                        oss << std::setw(width) << std::hex << std::setfill('0') << castedData.Value.UInt32;
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                switch (castedData.ValueType)
                                {
                                    case jarvis::dt_e::UINT8:
                                        oss << std::setw(width) << std::uppercase << std::hex << std::setfill('0') << castedData.Value.UInt8;
                                        break;
                                    case jarvis::dt_e::UINT16:
                                        oss << std::setw(width) << std::uppercase << std::hex << std::setfill('0') << castedData.Value.UInt16;
                                        break;
                                    case jarvis::dt_e::UINT32:
                                        oss << std::setw(width) << std::uppercase << std::hex << std::setfill('0') << castedData.Value.UInt32;
                                        break;
                                    default:
                                        break;
                                }
                            }
                            oss << std::dec; // Reset to decimal base
                        }
                        break;
                    
                    default:
                        LOG_ERROR(logger, "Type mismatch or unsupported format specifier.");
                        break;
                }
                ++i; // Skip the specifier
            }
            else
            {
                oss << format[i];
            }
        }

        return oss.str();
    }

    void Variable::Update(const std::vector<poll_data_t>& polledData)
    {
        removeOldestHistory();
        
        /**
         * @todo HasStatus, HasTimestamp 속성은 필요 없을 수 있습니다.
         *       고민해보고 필요 없다고 판단되면 삭제해야 합니다.
         */
        var_data_t variableData;
        variableData.StatusCode     = Status::Code::GOOD;
        variableData.Timestamp      = polledData.front().Timestamp;
        variableData.HasValue       = true;
        variableData.HasStatus      = true;
        variableData.HasTimestamp   = true;
        
        for (const auto& polledDatum : polledData)
        {
            if (polledDatum.StatusCode != Status::Code::GOOD)
            {
                variableData.StatusCode = polledDatum.StatusCode;
                break;
            }

            if (variableData.Timestamp != polledDatum.Timestamp)
            {
                variableData.StatusCode = Status::Code::BAD_INVALID_TIMESTAMP;
                variableData.HasTimestamp = false;
                break;
            }
        }
    

        implUpdate(polledData, &variableData);
        if (variableData.DataType == jarvis::dt_e::BOOLEAN || variableData.DataType == jarvis::dt_e::STRING)
        {
            goto CHECK_EVENT;
        }

        if (mBitIndex.first == true)
        {
            applyBitIndex(variableData);
            if (mMapMappingRules.first == true)
            {
                applyMappingRules(variableData);
                goto CHECK_EVENT;
            }
            goto CHECK_EVENT;
        }

        if (mMapMappingRules.first == true)
        {
            applyMappingRules(variableData);
            goto CHECK_EVENT;
        }

        if (mNumericScale.first == true)
        {
            applyNumericScale(variableData);
        }

        if (mNumericOffset.first == true)
        {
            applyNumericOffset(variableData);
        }


    CHECK_EVENT:
        if (variableData.StatusCode != Status::Code::GOOD)
        {
            variableData.HasValue = false;

            if (mDataBuffer.size() == 0)
            {
                variableData.IsEventType  = true;
                variableData.HasNewEvent  = true;
            }
            else
            {
                const var_data_t lastestHistory = mDataBuffer.back();
                variableData.IsEventType  = (lastestHistory.StatusCode != variableData.StatusCode);
                variableData.HasNewEvent  = (lastestHistory.StatusCode != variableData.StatusCode);
            }
            goto EMPLACE_DATA;
        }

        variableData.HasNewEvent = isEventOccured(variableData);
        variableData.IsEventType = variableData.HasNewEvent;
        if (variableData.HasNewEvent == true)
        {
            if (mDeprecableUID.substr(0, 2) == "DI" ||
                mDeprecableUID.substr(0, 2) == "DO" ||
                mDeprecableUID.substr(0, 1) == "P")
            {
                daq_struct_t daq;
                daq.Name = mDeprecableDisplayName;
                daq.SourceTimestamp = variableData.Timestamp;
                daq.Uid = mDeprecableUID;
                daq.Unit = mDeprecableDisplayUnit;
                daq.Topic = mDeprecableUID.substr(0, 2) == "DI" ? mqtt::topic_e::DAQ_INPUT  :
                            mDeprecableUID.substr(0, 2) == "DO" ? mqtt::topic_e::DAQ_OUTPUT :
                            mqtt::topic_e::DAQ_PARAM;
    
                switch (variableData.DataType)
                {
                case jarvis::dt_e::BOOLEAN:
                    daq.Value = variableData.Value.Boolean ? "true" : "false";
                    break;
                case jarvis::dt_e::FLOAT32 :
                    daq.Value = Float32ConvertToString(variableData.Value.Float32);
                    break;
                case jarvis::dt_e::FLOAT64:
                    daq.Value = Float64ConvertToString(variableData.Value.Float64);
                    break;
                case jarvis::dt_e::INT16:
                    daq.Value = std::to_string(variableData.Value.Int16);
                    break;
                case jarvis::dt_e::INT32:
                    daq.Value = std::to_string(variableData.Value.Int32);
                    break;
                case jarvis::dt_e::INT64:
                    daq.Value = std::to_string(variableData.Value.Int64);
                    break;
                case jarvis::dt_e::INT8 :
                    daq.Value = std::to_string(variableData.Value.Int8);
                    break;
                case jarvis::dt_e::STRING:
                    daq.Value = std::string(variableData.Value.String.Data);
                    break;
                case jarvis::dt_e::UINT16:
                    daq.Value = std::to_string(variableData.Value.UInt16);
                    break;
                case jarvis::dt_e::UINT32:
                    daq.Value = std::to_string(variableData.Value.UInt32);
                    break;
                case jarvis::dt_e::UINT64:
                    daq.Value = std::to_string(variableData.Value.UInt64);
                    break;
                case jarvis::dt_e::UINT8:
                    daq.Value = std::to_string(variableData.Value.UInt8);
                    break;
                default:
                    break;
                }
            
            
                JSON json;
                const std::string payload = json.Serialize(daq);
                mqtt::Message message(daq.Topic, payload);

                mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                cdo.Store(message);

            #if defined(DEBUG)
                LOG_DEBUG(logger,"[NodeId: %s][UID: %s]: NEW EVENT", mNodeID.c_str(), mDeprecableUID.c_str());
            #endif
            }
        }


    EMPLACE_DATA:
        try
        {
            mDataBuffer.emplace_back(variableData);
            // logData(variableData);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE DATA: %s", e.what());
        }
    }

    void Variable::logData(const var_data_t& data)
    {
        switch (data.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %s", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Boolean == true ? "true" : "false");
            break;
        case jarvis::dt_e::INT8:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %d", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Int8);
            break;
        case jarvis::dt_e::UINT8:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %u", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.UInt8);
            break;
        case jarvis::dt_e::INT16:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %d", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Int16);
            break;
        case jarvis::dt_e::UINT16:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %u", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.UInt16);
            break;
        case jarvis::dt_e::INT32:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %d", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Int32);
            break;
        case jarvis::dt_e::UINT32:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %u", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.UInt32);
            break;
        case jarvis::dt_e::INT64:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %lld", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Int64);
            break;
        case jarvis::dt_e::UINT64:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %llu", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.UInt64);
            break;
        case jarvis::dt_e::FLOAT32:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %.3f", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Float32);
            break;
        case jarvis::dt_e::FLOAT64:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %.3f", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.Float64);
            break;
        case jarvis::dt_e::STRING:
            LOG_INFO(logger,"[NodeID: %s][UID: %s]: %s", mNodeID.c_str(), mDeprecableUID.c_str(), data.Value.String.Data);
            break;
        default:
            break;
        }
    }


    void Variable::implUpdate(const std::vector<poll_data_t>& polledData, var_data_t* variableData)
    {
        if (mVectorDataTypes.size() == 1)
        {
            if (mVectorDataTypes.front() == jarvis::dt_e::BOOLEAN)
            {
                ASSERT((polledData.size() == 1), "BOOLEAN DATA TYPE IS ONLY APPLIED TO ONLY ONE DATUM POLLED FROM MACHINE");

                variableData->DataType = jarvis::dt_e::BOOLEAN;
                variableData->Value.Boolean = polledData.front().Value.Boolean;

                if (mMapMappingRules.first == true)
                {
                    applyMappingRules(*variableData);
                }
                return;
            }

            if (mVectorDataUnitOrders.first == true)
            {
                ASSERT((mVectorDataUnitOrders.second.size() == 1), "ONLY ONE DATA UNIT ORDER IS ALLOWED WHEN SINGULAR DATA TYPE IS PROVIDED");

                std::vector<casted_data_t> vectorCastedData;
                castWithDataUnitOrder(polledData, &vectorCastedData);

                variableData->DataType  = vectorCastedData.front().ValueType;
                variableData->Value     = vectorCastedData.front().Value;
                return;
            }
            else
            {
                casted_data_t castedData;
                castWithoutDataUnitOrder(polledData, &castedData);
        
                variableData->DataType  = castedData.ValueType;
                variableData->Value     = castedData.Value;
                
                return;
            }
        }
        else
        {
            std::vector<casted_data_t> vectorCastedData;
            castWithDataUnitOrder(polledData, &vectorCastedData);

            std::string formattedString = createFormattedString(mFormatString.second.c_str(), vectorCastedData);
            variableData->DataType      = jarvis::dt_e::STRING;
            variableData->Value.String  = ToMuffinString(formattedString);
            return;
        }
    }
    
    void Variable::removeOldestHistory()
    {
        if (mDataBuffer.size() == mMaxHistorySize)
        {
            auto it = mDataBuffer.begin();
            if (it->DataType == jarvis::dt_e::STRING)
            {
                delete it->Value.String.Data;
                it->Value.String.Data = nullptr;
            }
            mDataBuffer.pop_front();
        }
    }
    
    void Variable::flattenToByteArray(const std::vector<poll_data_t>& polledData, std::vector<uint8_t>* outputFlattenVector)
    {
        ASSERT((outputFlattenVector != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        ASSERT((outputFlattenVector->empty() == true), "OUTPUT PARAMETER MUST BE AN EMPTY VECTOR");

        for (auto& polledDatum : polledData)
        {
            switch (polledDatum.ValueType)
            {
            case jarvis::dt_e::INT8:
            case jarvis::dt_e::UINT8:
                outputFlattenVector->emplace_back(polledDatum.Value.UInt8);
                break;

            case jarvis::dt_e::INT16:
            case jarvis::dt_e::UINT16:
                {
                    const uint8_t byteHigh  = static_cast<uint8_t>(((polledDatum.Value.UInt16 >> 8) & 0xFF));
                    const uint8_t byteLow   = static_cast<uint8_t>((polledDatum.Value.UInt16 & 0xFF));
                    
                    outputFlattenVector->emplace_back(byteHigh);
                    outputFlattenVector->emplace_back(byteLow);
                }
                break;

            /**
             * @todo 기계에서 수집한 데이터의 크기가 32bit, 64bit인 경우를 구현해야 합니다.
             */
            default:
                ASSERT(false, "UNDEFINED DATA TYPE TO FLATTEN AS A BYTE ARRAY");
                break;
            }
        }
    }
    
    void Variable::castByteVector(const jarvis::dt_e dataType, std::vector<uint8_t>& vectorBytes, casted_data_t* castedData)
    {
        switch (dataType)
        {
        case jarvis::dt_e::INT8:
            castedData->ValueType = jarvis::dt_e::INT8;
            ASSERT((vectorBytes.size() == 1), "BYTE ARRAY SIZE MUST BE EQUAL TO 1");
            castedData->Value.UInt16 = static_cast<uint8_t>(vectorBytes[0]);
            break;
        
        case jarvis::dt_e::INT16:
            castedData->ValueType = jarvis::dt_e::INT16;
            ASSERT((vectorBytes.size() == 2), "BYTE ARRAY SIZE MUST BE EQUAL TO 2");
            castedData->Value.Int16 = 
                static_cast<int16_t>(vectorBytes[0]) <<  8 | 
                vectorBytes[1];
            break;
        
        case jarvis::dt_e::INT32:
            castedData->ValueType = jarvis::dt_e::INT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.Int32 = 
                static_cast<int32_t>(vectorBytes[0]) << 24 | 
                static_cast<int32_t>(vectorBytes[1]) << 16 | 
                static_cast<int32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        
        case jarvis::dt_e::INT64:
            castedData->ValueType = jarvis::dt_e::INT64;
            ASSERT((vectorBytes.size() == 8), "BYTE ARRAY SIZE MUST BE EQUAL TO 8");
            castedData->Value.Int64 = 
                static_cast<int64_t>(vectorBytes[0]) << 56 | 
                static_cast<int64_t>(vectorBytes[1]) << 48 | 
                static_cast<int64_t>(vectorBytes[2]) << 40 | 
                static_cast<int64_t>(vectorBytes[3]) << 32 | 
                static_cast<int64_t>(vectorBytes[4]) << 24 | 
                static_cast<int64_t>(vectorBytes[5]) << 16 | 
                static_cast<int64_t>(vectorBytes[6]) <<  8 | 
                vectorBytes[7];
            break;
        
        case jarvis::dt_e::UINT8:
            castedData->ValueType = jarvis::dt_e::UINT8;
            ASSERT((vectorBytes.size() == 1), "BYTE ARRAY SIZE MUST BE EQUAL TO 1");
            castedData->Value.UInt16 = static_cast<uint8_t>(vectorBytes[0]);
            break;
        
        case jarvis::dt_e::UINT16:
            castedData->ValueType = jarvis::dt_e::UINT16;
            ASSERT((vectorBytes.size() == 2), "BYTE ARRAY SIZE MUST BE EQUAL TO 2");
            castedData->Value.UInt16 = 
                static_cast<uint16_t>(vectorBytes[0]) <<  8 | 
                vectorBytes[1];
            break;
        case jarvis::dt_e::UINT32:
        {
            castedData->ValueType = jarvis::dt_e::UINT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.UInt32 = 
                static_cast<int32_t>(vectorBytes[0]) << 24 | 
                static_cast<int32_t>(vectorBytes[1]) << 16 | 
                static_cast<int32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        }
        
        case jarvis::dt_e::UINT64:
            castedData->ValueType = jarvis::dt_e::UINT64;
            ASSERT((vectorBytes.size() == 8), "BYTE ARRAY SIZE MUST BE EQUAL TO 8");
            castedData->Value.UInt64 = 
                static_cast<uint64_t>(vectorBytes[0]) << 56 | 
                static_cast<uint64_t>(vectorBytes[1]) << 48 | 
                static_cast<uint64_t>(vectorBytes[2]) << 40 | 
                static_cast<uint64_t>(vectorBytes[3]) << 32 | 
                static_cast<uint64_t>(vectorBytes[4]) << 24 | 
                static_cast<uint64_t>(vectorBytes[5]) << 16 | 
                static_cast<uint64_t>(vectorBytes[6]) <<  8 | 
                vectorBytes[7];
            break;
        
        case jarvis::dt_e::FLOAT32:
            castedData->ValueType = jarvis::dt_e::FLOAT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.UInt32 = 
                static_cast<uint32_t>(vectorBytes[0]) << 24 | 
                static_cast<uint32_t>(vectorBytes[1]) << 16 | 
                static_cast<uint32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        
        case jarvis::dt_e::FLOAT64:
            castedData->ValueType = jarvis::dt_e::FLOAT64;
            ASSERT((vectorBytes.size() == 8), "BYTE ARRAY SIZE MUST BE EQUAL TO 8");
            castedData->Value.UInt64 = 
                static_cast<uint64_t>(vectorBytes[0]) << 56 | 
                static_cast<uint64_t>(vectorBytes[1]) << 48 | 
                static_cast<uint64_t>(vectorBytes[2]) << 40 | 
                static_cast<uint64_t>(vectorBytes[3]) << 32 | 
                static_cast<uint64_t>(vectorBytes[4]) << 24 | 
                static_cast<uint64_t>(vectorBytes[5]) << 16 | 
                static_cast<uint64_t>(vectorBytes[6]) <<  8 | 
                vectorBytes[7];
            break;
        
        case jarvis::dt_e::STRING:
        {
            castedData->ValueType = jarvis::dt_e::STRING;
            std::string string(vectorBytes.begin(), vectorBytes.end());
            castedData->Value.String = ToMuffinString(string);
            break;
        }
        
        default:
            break;
        }
    }

    void Variable::applyBitIndex(var_data_t& variableData)
    {
        switch (variableData.DataType)
        {
        case jarvis::dt_e::INT8:
        case jarvis::dt_e::UINT8:
            variableData.Value.Boolean = (variableData.Value.UInt8 >> mBitIndex.second) & 1;
            break;
        case jarvis::dt_e::INT16:
        case jarvis::dt_e::UINT16:
            variableData.Value.Boolean = (variableData.Value.UInt16 >> mBitIndex.second) & 1;
            break;
        case jarvis::dt_e::INT32:
        case jarvis::dt_e::UINT32:
            variableData.Value.Boolean = (variableData.Value.UInt32 >> mBitIndex.second) & 1;
            break;
        case jarvis::dt_e::INT64:
        case jarvis::dt_e::UINT64:
            variableData.Value.Boolean = (variableData.Value.UInt64 >> mBitIndex.second) & 1;
            break;
        default:
            break;
        }

        variableData.DataType  = jarvis::dt_e::BOOLEAN;
    }

    void Variable::applyMappingRules(var_data_t& variableData)
    {
        auto it = mMapMappingRules.second.end();

        switch (variableData.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            it = mMapMappingRules.second.find(variableData.Value.Boolean);
            break;
        case jarvis::dt_e::INT8:
        case jarvis::dt_e::UINT8:
            it = mMapMappingRules.second.find(variableData.Value.UInt8);
            break;
        case jarvis::dt_e::INT16:
        case jarvis::dt_e::UINT16:
            it = mMapMappingRules.second.find(variableData.Value.UInt16);
            break;
        case jarvis::dt_e::INT32:
        case jarvis::dt_e::UINT32:
            it = mMapMappingRules.second.find(variableData.Value.UInt16);
            break;
        default:
            break;
        }
        ASSERT((it != mMapMappingRules.second.end()), "END ITERATOR IS NOT ALLOWED WHEN APPLYING MAPPING RULES");
        
        variableData.DataType = jarvis::dt_e::STRING;
        variableData.Value.String = ToMuffinString(it->second);
    }

    void Variable::applyNumericScale(var_data_t& variableData)
    {
        const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
        const double denominator = pow(10, exponent);

        switch (variableData.DataType)
        {
        case jarvis::dt_e::INT8:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int8) * denominator;
            break;
        
        case jarvis::dt_e::UINT8:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt8) * denominator;
            break;
        
        case jarvis::dt_e::INT16:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int16) * denominator;
            break;
        
        case jarvis::dt_e::UINT16:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt16) * denominator;
            break;
        
        case jarvis::dt_e::INT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<float>(variableData.Value.Int32) * denominator;
            break;
        
        case jarvis::dt_e::UINT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<float>(variableData.Value.UInt32) * denominator;
            break;
        
        case jarvis::dt_e::FLOAT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Float32) * denominator;
            break;
        
        case jarvis::dt_e::INT64:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int64) * denominator;
            break;
        
        case jarvis::dt_e::UINT64:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt64) * denominator;
            break;
        
        case jarvis::dt_e::FLOAT64:
            variableData.Value.Float64 = variableData.Value.Float64 * denominator;
            break;
        
        default:
            break;
        }
    }

    void Variable::applyNumericOffset(var_data_t& variableData)
    {
        switch (variableData.DataType)
        {
        case jarvis::dt_e::INT8:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int8) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::UINT8:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt8) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::INT16:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int16) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::UINT16:
            variableData.DataType = jarvis::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt16) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::INT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int32) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::UINT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt32) - mNumericOffset.second;
            break;
        
        case jarvis::dt_e::FLOAT32:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Float32) - mNumericOffset.second;
            break;

        case jarvis::dt_e::INT64:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int64) - mNumericOffset.second;
            break;

        case jarvis::dt_e::UINT64:
            variableData.DataType = jarvis::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt64) - mNumericOffset.second;
            break;
            
        case jarvis::dt_e::FLOAT64:
            variableData.Value.Float64 = variableData.Value.Float64 - mNumericOffset.second;
            break;

        default:
            break;
        }
    }

    void Variable::castWithDataUnitOrder(const std::vector<poll_data_t>& polledData, std::vector<casted_data_t>* outputCastedData)
    {
        ASSERT((outputCastedData != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        ASSERT((outputCastedData->empty() == true), "OUTPUT PARAMETER MUST BE AN EMPTY ARRAY");

        uint8_t dataTypeIndex = 0;

        /**
         * @brief 현재는 기계에서 수집한 데이터의 타입이 16비트일 때까지만 구현되어 있습니다.
         *        향후 다른 프로토콜이 필요하므로 나머지 데이터 타입의 처리를 구현해야 합니다.
         */
        for (auto& dataUnitOrders : mVectorDataUnitOrders.second)
        {
            std::vector<uint8_t> vectorFlattened;
            flattenToByteArray(polledData, &vectorFlattened);

            const uint8_t totalBytes = sizeof(uint8_t) * vectorFlattened.size();
            std::vector<uint8_t> arrayOrderedBytes;
            arrayOrderedBytes.reserve(totalBytes);            

            /**
             * @todo 32bit, 64bit인 경우를 구현해야 합니다.
             */
            for (auto& dataUnitOrder : dataUnitOrders)
            {
                const uint8_t startByteIndex  = 2 * dataUnitOrder.Index;
                const uint8_t finishByteIndex = startByteIndex + 1;

                if (dataUnitOrder.DataUnit == jarvis::data_unit_e::WORD)
                {
                    arrayOrderedBytes.emplace_back(vectorFlattened[startByteIndex]);
                    arrayOrderedBytes.emplace_back(vectorFlattened[finishByteIndex]);
                }
                else if (dataUnitOrder.DataUnit == jarvis::data_unit_e::BYTE)
                {
                    if (dataUnitOrder.ByteOrder == jarvis::byte_order_e::HIGHER)
                    {
                        arrayOrderedBytes.emplace_back(vectorFlattened[startByteIndex]);
                    }
                    else
                    {
                        arrayOrderedBytes.emplace_back(vectorFlattened[finishByteIndex]);
                    }
                }

                if (mDeprecableUID == "DI1E")
                {
                    LOG_DEBUG(logger, "[DI1E] 0x%X 0x%X", vectorFlattened[startByteIndex], vectorFlattened[finishByteIndex]);
                }
            }

            casted_data_t castedData;
            castByteVector(mVectorDataTypes[dataTypeIndex], arrayOrderedBytes, &castedData);
            outputCastedData->emplace_back(castedData);
            ++dataTypeIndex;
        }
    }

    void Variable::castWithoutDataUnitOrder(const std::vector<poll_data_t>& polledData, casted_data_t* outputCastedData)
    {
        ASSERT((outputCastedData != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        std::vector<uint8_t> vectorFlattened;
        flattenToByteArray(polledData, &vectorFlattened);
        castByteVector(mVectorDataTypes.front(), vectorFlattened, outputCastedData);

        if (mModbusArea.first == true && outputCastedData->ValueType == jarvis::dt_e::STRING)
        {
            for (size_t i = 0; i < outputCastedData->Value.String.Length - 1; i += 2)
            {
                std::swap(outputCastedData->Value.String.Data[i], outputCastedData->Value.String.Data[i + 1]);
            }
        }
    }

    bool Variable::isEventOccured(var_data_t& variableData)
    {
        if (mHasAttributeEvent == false)
        {
            // LOG_INFO(logger,"HasAttributeEvent IS FALSE");
            return false;
        }

        if (mDataBuffer.size() == 0)
        {
            // 이벤트 데이터 초기값 전송을 위한 변수입니다. 
            if (mInitEvent)
            {   
                mInitEvent = false;
                return true;
            }
            
            LOG_INFO(logger,"mDataBuffer IS 0");
            return false;
        }

        const var_data_t lastestHistory = mDataBuffer.back();
        switch (variableData.DataType)
        {
        case jarvis::dt_e::INT8:
            return lastestHistory.Value.Int8 != variableData.Value.Int8;
        case jarvis::dt_e::UINT8:
            return lastestHistory.Value.UInt8 != variableData.Value.UInt8;
        case jarvis::dt_e::INT16:
            return lastestHistory.Value.Int16 != variableData.Value.Int16;
        case jarvis::dt_e::UINT16:
            return lastestHistory.Value.UInt16 != variableData.Value.UInt16;
        case jarvis::dt_e::INT32:
            return lastestHistory.Value.Int32 != variableData.Value.Int32;
        case jarvis::dt_e::UINT32:
            return lastestHistory.Value.UInt32 != variableData.Value.UInt32;
        case jarvis::dt_e::FLOAT32:
            return lastestHistory.Value.Float32 != variableData.Value.Float32;
        case jarvis::dt_e::INT64:
            return lastestHistory.Value.Int64 != variableData.Value.Int64;
        case jarvis::dt_e::UINT64:
            return lastestHistory.Value.UInt64 != variableData.Value.UInt64;
        case jarvis::dt_e::FLOAT64:
            return lastestHistory.Value.Float64 != variableData.Value.Float64;
        case jarvis::dt_e::STRING:
            return static_cast<bool>(strcmp(lastestHistory.Value.String.Data, variableData.Value.String.Data));
        default:
            return false;
        }
   
        return false;
    }

    string_t Variable::ToMuffinString(const std::string& stdString)
    {
        string_t string;
        string.Length = stdString.length();
        string.Data = new char[string.Length + 1];
        memset(string.Data, '\0', (string.Length + 1));

        for (size_t i = 0; i < string.Length; ++i)
        {
            string.Data[i] = stdString[i];
        }

        return string;
    }

    // void Variable::strategySingleDataType()
    // {
    //     ;
    // }


    size_t Variable::RetrieveCount() const
    {
        return mDataBuffer.size();
    }

    var_data_t Variable::RetrieveData() const
    {
        return mDataBuffer.back();
    }

    std::vector<var_data_t> Variable::RetrieveHistory(const size_t numberofHistory) const
    {
        ASSERT((numberofHistory < mMaxHistorySize + 1), "CANNOT RETRIEVE MORE THAN THE MAXIMUM HITORY SIZE");

        std::vector<var_data_t> history;

        auto it = mDataBuffer.end();
        for (size_t i = 0; i < numberofHistory; i++)
        {
            --it;
            history.emplace_back(it.operator*());
        }
        
        return history;
    }

    jarvis::addr_u Variable::GetAddress() const
    {
        return mAddress;
    }

    uint8_t Variable::GetQuantity() const
    {
        return mAddressQuantity.second;
    }

    uint16_t Variable::GetBitIndex() const
    {
        return mBitIndex.second;
    }

    jarvis::mb_area_e Variable::GetModbusArea() const
    {
        return mModbusArea.second;
    }

    std::pair<Status,uint16_t> Variable::ConvertModbusData(std::string& data)
    {
        if (data.empty() == true)
        {
            return std::make_pair(Status(Status::Code::BAD_NO_DATA), 0);
        }
        
        /**
         * @brief 현재 단일 레지스터나 비트만 제어 가능함, 추후 Method 개발시 업데이트 예정입니다.
         * 
         */
        if (mAddressQuantity.first == true && mAddressQuantity.second != 1)
        {
            LOG_ERROR(logger, "ASCII DATA IS NOT SUPPORTED YET");
            return std::make_pair(Status(Status::Code::BAD_SERVICE_UNSUPPORTED), 0);
        }
        
    
        if (mVectorDataTypes.at(0) != jarvis::dt_e::STRING)
        {
            // 서버에서 입력된 value가 문자열인지 판단하는 로직, 더 좋은 방법이 있나?
            bool decimalFound = false;
            size_t start = (data[0] == '-') ? 1 : 0;
            for (size_t i = start; i < data.length(); ++i) 
            {
                char c = data[i];
                if (c == '.') 
                {
                    if (decimalFound) 
                    {
                        return std::make_pair(Status(Status::Code::BAD_TYPE_MISMATCH), 0);
                    }
                    decimalFound = true;
                } 
                else if (!isdigit(c)) 
                {
                    return std::make_pair(Status(Status::Code::BAD_TYPE_MISMATCH), 0);
                }
            }

            float floatTemp = 0;
            
            if (mMapMappingRules.first == true)
            {
                auto it = mMapMappingRules.second.find(Convert.ToUInt16(data));
                if (it != mMapMappingRules.second.end()) 
                {
                    return std::make_pair(Status(Status::Code::GOOD), it->first);
                } 
                else
                {
                    LOG_ERROR(logger,"NO MATCHING KEY DATA IN MAPPING RULES, DATA : %s",data.c_str());
                    return std::make_pair(Status(Status::Code::BAD_NO_DATA_AVAILABLE), it->first);
                }
            }

            if (mBitIndex.first == true)
            {
                if (Convert.ToUInt16(data) > 1)
                {
                    LOG_ERROR(logger,"BIT DATA HAS ONLY 1 or 0 VALUE , DATA : %s",data.c_str());
                    return std::make_pair(Status(Status::Code::BAD_NO_DATA_AVAILABLE), 0);
                }
            }

            if (mNumericOffset.first == true)
            {
               floatTemp = Convert.ToFloat(data) - mNumericOffset.second;
               if (mNumericScale.first == false)
               {
                    //현재 구조에서 offset이 있는데 scale이 없는 경우가 있을지는 모르겠다.
                    return std::make_pair(Status(Status::Code::GOOD), static_cast<uint16_t>(floatTemp));
               }
            }

            if (mNumericScale.first == true)
            {
                if (mNumericOffset.first == true)
                {
                    const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
                    const double denominator = pow(10, exponent);
                    return std::make_pair(Status(Status::Code::GOOD), static_cast<uint16_t>(floatTemp/ denominator));
                }
                else
                {
                    const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
                    const double denominator = pow(10, exponent);
                    return std::make_pair(Status(Status::Code::GOOD), static_cast<uint16_t>(Convert.ToFloat(data)/ denominator));
                }
            }

            LOG_INFO(logger, "Raw data : %s, Convert Modbus data : %u" , data.c_str(), Convert.ToUInt16(data));
            return std::make_pair(Status(Status::Code::GOOD), Convert.ToUInt16(data));
        }
        else
        {
            LOG_ERROR(logger, "ASCII DATA IS NOT SUPPORTED YET");
            return std::make_pair(Status(Status::Code::BAD_SERVICE_UNSUPPORTED), 0);
        }
    }

    std::pair<bool, uint8_t> Variable::GetBitindex() const
    {
        return mBitIndex;
    }
    
    std::string Variable::Float32ConvertToString(const float& data) const
    {
        if (mNumericScale.first == false)
        {
            /**
             * @todo 현재 스케일이 없는 float일때 소수점2자리로 고정시켜 서버로 전송. 추후 수정해야함
             */
            char buffer[20];    
            sprintf(buffer, "%0.2f", data);
            return std::string(buffer);  

        }
        
        const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10];
        sprintf(format, "%%.%df", decimalPlaces);

        char buffer[20];
        sprintf(buffer, format, data);

        return std::string(buffer);
    }

    std::string Variable::Float64ConvertToString(const double& data) const
    {
        if (mNumericScale.first == false)
        {
            /**
             * @todo 현재 스케일이 없는 float일때 소수점2자리로 고정시켜 서버로 전송. 추후 수정해야함
             */
            char buffer[32];    
            sprintf(buffer, "%0.2f", data);
            return std::string(buffer); 
        }
        
        const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
        
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10];
        sprintf(format, "%%.%df", decimalPlaces);

        char buffer[20];
        sprintf(buffer, format, data);

        return std::string(buffer);
    }

    std::string Variable::FloatConvertToStringForLimitValue(const float& data) const
    {   
        if (mNumericScale.first == false)
        {
            if (mDataType == jarvis::dt_e::FLOAT32 || mDataType == jarvis::dt_e::FLOAT64)
            {
                /**
                * @todo 현재 스케일이 없는 float일때 소수점2자리로 고정시켜 서버로 전송. 추후 수정해야함
                */
                char buffer[32];    
                sprintf(buffer, "%0.2f", data);
                return std::string(buffer); 
            }
            else
            {
                uint16_t returnData = (uint16_t)data;
                return std::to_string(returnData);
            }
        }
        
        const int8_t exponent = static_cast<int8_t>(mNumericScale.second);
        
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10];
        sprintf(format, "%%.%df", decimalPlaces);

        char buffer[20];
        sprintf(buffer, format, data);

        return std::string(buffer);
    }

    std::pair<bool, daq_struct_t> Variable::CreateDaqStruct()
    {
        daq_struct_t daq;

        if (RetrieveCount() == 0)
        {
            return std::make_pair(false, daq);
        }
    
        var_data_t variableData = RetrieveData();
        
        daq.Name = mDeprecableDisplayName;
        daq.SourceTimestamp = variableData.Timestamp;
        daq.Uid = mDeprecableUID;
        daq.Unit = mDeprecableDisplayUnit;
        daq.Topic = mDeprecableUID.substr(0, 2) == "DI" ? mqtt::topic_e::DAQ_INPUT  :
                    mDeprecableUID.substr(0, 2) == "DO" ? mqtt::topic_e::DAQ_OUTPUT :
                    mqtt::topic_e::DAQ_PARAM;

        switch (variableData.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            daq.Value = variableData.Value.Boolean ? "true" : "false";
            break;
        case jarvis::dt_e::FLOAT32 :
            daq.Value = Float32ConvertToString(variableData.Value.Float32);
            break;
        case jarvis::dt_e::FLOAT64:
            daq.Value = Float64ConvertToString(variableData.Value.Float64);
            break;
        case jarvis::dt_e::INT16:
            daq.Value = std::to_string(variableData.Value.Int16);
            break;
        case jarvis::dt_e::INT32:
            daq.Value = std::to_string(variableData.Value.Int32);
            break;
        case jarvis::dt_e::INT64:
            daq.Value = std::to_string(variableData.Value.Int64);
            break;
        case jarvis::dt_e::INT8 :
            daq.Value = std::to_string(variableData.Value.Int8);
            break;
        case jarvis::dt_e::STRING:
            daq.Value = std::string(variableData.Value.String.Data);
            break;
        case jarvis::dt_e::UINT16:
            daq.Value = std::to_string(variableData.Value.UInt16);
            break;
        case jarvis::dt_e::UINT32:
            daq.Value = std::to_string(variableData.Value.UInt32);
            break;
        case jarvis::dt_e::UINT64:
            daq.Value = std::to_string(variableData.Value.UInt64);
            break;
        case jarvis::dt_e::UINT8:
            daq.Value = std::to_string(variableData.Value.UInt8);
            break;
        default:
            break;
        }

        return std::make_pair(true, daq);
    }


    uint32_t Variable::mSamplingIntervalInMillis = 1000;
}}