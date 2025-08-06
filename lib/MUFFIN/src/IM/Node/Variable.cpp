/**
 * @file Variable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 정의합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <cmath>
#include <iomanip>
#include <sstream>
#include <string.h>

#include "Common/Assert.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Protocol/MQTT/CDO.h"
#include "Variable.h"



namespace muffin { namespace im {

    Variable::Variable(const jvs::config::Node* cin)
        : mCIN(cin)
    {
        if (mCIN->GetFormatString().first == Status::Code::GOOD)
        {
            mDataType = jvs::dt_e::STRING;
        }
        else
        {
            ASSERT((mCIN->GetDataTypes().second.size() == 1), "DATA TYPE VECTOR SIZE MUST BE 1 WHEN FORMAT STRING IS DISABLED");
            mDataType = mCIN->GetDataTypes().second[0];
        }
    }

    const char* Variable::GetNodeID() const
    {
        return mCIN->GetNodeID().second;
    }

    jvs::addr_u Variable::GetAddress() const
    {
        return mCIN->GetAddrress().second;
    }

    uint8_t Variable::GetQuantity() const
    {
        return mCIN->GetNumericAddressQuantity().second;
    }

    std::vector<std::array<uint16_t, 2>> Variable::GetArrayIndex() const
    {
        return mCIN->GetArrayIndex().first == Status::Code::GOOD ?
            mCIN->GetArrayIndex().second : 
            std::vector<std::array<uint16_t, 2>>{};
    }

    int16_t Variable::GetBitIndex() const
    {
        return mCIN->GetBitIndex().first == Status::Code::GOOD ?
            mCIN->GetBitIndex().second : 
            -1;
    }

    jvs::node_area_e Variable::GetNodeArea() const
    {
        return mCIN->GetNodeArea().second;
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
                                case jvs::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jvs::dt_e::INT64:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int64;
                                    break;
                                case jvs::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT32:
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
                                case jvs::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jvs::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT16:
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
                                case jvs::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.Int32;
                                    break;
                                case jvs::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.UInt32;
                                    break;
                                case jvs::dt_e::UINT64:
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
                                case jvs::dt_e::INT8:
                                {
                                    const int32_t value = castedData.Value.Int8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::INT16:
                                {
                                    const int32_t value = castedData.Value.Int16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT8:
                                {
                                    const int32_t value = castedData.Value.UInt8;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT16:
                                {
                                    const int32_t value = castedData.Value.UInt16;
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << value;
                                    break;
                                }
                                case jvs::dt_e::UINT32:
                                    oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.UInt32;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    
                    case 'f':
                        if (castedData.ValueType == jvs::dt_e::FLOAT32)
                        {
                            oss << std::fixed;
                            if (precision >= 0)
                            {
                                oss << std::setprecision(precision);
                            }
                            oss << std::setw(width) << castedData.Value.Float32;
                        }
                        else if (castedData.ValueType == jvs::dt_e::FLOAT64)
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
                        if (castedData.ValueType == jvs::dt_e::STRING)
                        {
                            oss << std::setw(width) << (hasZeroPadding ? std::setfill('0') : std::setfill(' ')) << castedData.Value.String.Data;
                        }
                        break;
                    
                    case 'x':
                    case 'X':
                        if (jvs::dt_e::BOOLEAN < castedData.ValueType && castedData.ValueType < jvs::dt_e::FLOAT32)
                        {
                            if (format[i + 1] == 'x')
                            {
                                switch (castedData.ValueType)
                                {
                                    case jvs::dt_e::UINT8:
                                        oss << std::setw(width) << std::hex << std::setfill('0') << castedData.Value.UInt8;
                                        break;
                                    case jvs::dt_e::UINT16:
                                        oss << std::setw(width) << std::hex << std::setfill('0') << castedData.Value.UInt16;
                                        break;
                                    case jvs::dt_e::UINT32:
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
                                    case jvs::dt_e::UINT8:
                                        oss << std::setw(width) << std::uppercase << std::hex << std::setfill('0') << castedData.Value.UInt8;
                                        break;
                                    case jvs::dt_e::UINT16:
                                        oss << std::setw(width) << std::uppercase << std::hex << std::setfill('0') << castedData.Value.UInt16;
                                        break;
                                    case jvs::dt_e::UINT32:
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

        std::string result = oss.str();
        oss.str("");
        oss.clear();
        return result;
    }

    void Variable::UpdateError()
    {
        removeOldestHistory();

        var_data_t variableData;
        variableData.StatusCode     = Status::Code::BAD;
        variableData.Timestamp      = 0;
        variableData.HasValue       = false;
        variableData.HasStatus      = false;
        variableData.HasTimestamp   = false;

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

        variableData.HasNewEvent = isEventOccured(variableData);
        variableData.IsEventType = variableData.HasNewEvent;
        mHasNewEvent = variableData.HasNewEvent;
        
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
                
                LOG_WARNING(logger,"variableData.Timestamp : %llu, polledDatum.Timestamp : %llu",variableData.Timestamp,polledDatum.Timestamp);
                variableData.StatusCode = Status::Code::BAD_INVALID_TIMESTAMP;
                variableData.HasTimestamp = false;
                break;
            }
        }
    

        implUpdate(polledData, &variableData);
        if (variableData.DataType == jvs::dt_e::BOOLEAN || 
            variableData.DataType == jvs::dt_e::STRING  ||
            variableData.DataType == jvs::dt_e::ARRAY
        )
        {
            goto CHECK_EVENT;
        }

        if (mCIN->GetBitIndex().first == Status::Code::GOOD)
        {
            applyBitIndex(variableData);
            goto CHECK_EVENT;
        }

        if (mCIN->GetNumericScale().first == Status::Code::GOOD)
        {
            applyNumericScale(variableData);
        }

        if (mCIN->GetNumericOffset().first == Status::Code::GOOD)
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

        }

        variableData.HasNewEvent = isEventOccured(variableData);
        variableData.IsEventType = variableData.HasNewEvent;
        mHasNewEvent = variableData.HasNewEvent;
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

    void Variable::implUpdate(const std::vector<poll_data_t>& polledData, var_data_t* variableData)
    {
        if (mCIN->GetDataTypes().second.front() == jvs::dt_e::ARRAY)
        {
            variableData->ArrayValue.reserve(polledData.size());
            variableData->ArrayDataType = polledData.at(0).ValueType;
            variableData->DataType = jvs::dt_e::ARRAY;
            for (auto& datum : polledData)
            {
                variableData->ArrayValue.emplace_back(datum.Value);
            }
            return;
        }
        
        if (mCIN->GetDataTypes().second.size() == 1)
        {
            if (mCIN->GetDataTypes().second.front() == jvs::dt_e::BOOLEAN)
            {
                ASSERT((polledData.size() == 1), "BOOLEAN DATA TYPE IS ONLY APPLIED TO ONLY ONE DATUM POLLED FROM MACHINE");

                variableData->DataType = jvs::dt_e::BOOLEAN;
                variableData->Value.Boolean = polledData.front().Value.Boolean;
                return;
            }

            if (mCIN->GetDataUnitOrders().first == Status::Code::GOOD)
            {
                ASSERT((mCIN->GetDataUnitOrders().second.size() == 1), "ONLY ONE DATA UNIT ORDER IS ALLOWED WHEN SINGULAR DATA TYPE IS PROVIDED");

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

            std::string formattedString = createFormattedString(mCIN->GetFormatString().second.c_str(), vectorCastedData);
            variableData->DataType      = jvs::dt_e::STRING;
            variableData->Value.String  = ToMuffinString(formattedString);
            return;
        }
    }
    
    void Variable::removeOldestHistory()
    {
        if (mDataBuffer.size() == MAX_HISTORY_SIZE)
        {
            auto it = mDataBuffer.begin();
            if (it->DataType == jvs::dt_e::STRING)
            {
                memset(it->Value.String.Data, '\0', sizeof(it->Value.String.Data));
            }
            mDataBuffer.erase(it);
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
            case jvs::dt_e::INT8:
            case jvs::dt_e::UINT8:
                outputFlattenVector->emplace_back(polledDatum.Value.UInt8);
                break;

            case jvs::dt_e::INT16:
            case jvs::dt_e::UINT16:
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
    
    void Variable::castByteVector(const jvs::dt_e dataType, std::vector<uint8_t>& vectorBytes, casted_data_t* castedData)
    {   
        switch (dataType)
        {
        case jvs::dt_e::INT8:
            castedData->ValueType = jvs::dt_e::INT8;
            ASSERT((vectorBytes.size() == 1), "BYTE ARRAY SIZE MUST BE EQUAL TO 1");
            castedData->Value.UInt16 = static_cast<uint8_t>(vectorBytes[0]);
            break;
        
        case jvs::dt_e::INT16:
            castedData->ValueType = jvs::dt_e::INT16;
            ASSERT((vectorBytes.size() == 2), "BYTE ARRAY SIZE MUST BE EQUAL TO 2");
            castedData->Value.Int16 = 
                static_cast<int16_t>(vectorBytes[0]) <<  8 | 
                vectorBytes[1];
            break;
        
        case jvs::dt_e::INT32:
            castedData->ValueType = jvs::dt_e::INT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.Int32 = 
                static_cast<int32_t>(vectorBytes[0]) << 24 | 
                static_cast<int32_t>(vectorBytes[1]) << 16 | 
                static_cast<int32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        
        case jvs::dt_e::INT64:
            castedData->ValueType = jvs::dt_e::INT64;
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
        
        case jvs::dt_e::UINT8:
            castedData->ValueType = jvs::dt_e::UINT8;
            ASSERT((vectorBytes.size() == 1), "BYTE ARRAY SIZE MUST BE EQUAL TO 1");
            castedData->Value.UInt16 = static_cast<uint8_t>(vectorBytes[0]);
            break;
        
        case jvs::dt_e::UINT16:
            castedData->ValueType = jvs::dt_e::UINT16;
            ASSERT((vectorBytes.size() == 2), "BYTE ARRAY SIZE MUST BE EQUAL TO 2");
            castedData->Value.UInt16 = 
                static_cast<uint16_t>(vectorBytes[0]) <<  8 | 
                vectorBytes[1];
            break;
        case jvs::dt_e::UINT32:
        {
            castedData->ValueType = jvs::dt_e::UINT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.UInt32 = 
                static_cast<int32_t>(vectorBytes[0]) << 24 | 
                static_cast<int32_t>(vectorBytes[1]) << 16 | 
                static_cast<int32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        }
        
        case jvs::dt_e::UINT64:
            castedData->ValueType = jvs::dt_e::UINT64;
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
        
        case jvs::dt_e::FLOAT32:
            castedData->ValueType = jvs::dt_e::FLOAT32;
            ASSERT((vectorBytes.size() == 4), "BYTE ARRAY SIZE MUST BE EQUAL TO 4");
            castedData->Value.UInt32 = 
                static_cast<uint32_t>(vectorBytes[0]) << 24 | 
                static_cast<uint32_t>(vectorBytes[1]) << 16 | 
                static_cast<uint32_t>(vectorBytes[2]) <<  8 | 
                vectorBytes[3];
            break;
        
        case jvs::dt_e::FLOAT64:
            castedData->ValueType = jvs::dt_e::FLOAT64;
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
        
        case jvs::dt_e::STRING:
        {
            castedData->ValueType = jvs::dt_e::STRING;
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
        case jvs::dt_e::INT8:
        case jvs::dt_e::UINT8:
            variableData.Value.Boolean = (variableData.Value.UInt8 >> mCIN->GetBitIndex().second) & 1;
            break;
        case jvs::dt_e::INT16:
        case jvs::dt_e::UINT16:
            variableData.Value.Boolean = (variableData.Value.UInt16 >> mCIN->GetBitIndex().second) & 1;
            break;
        case jvs::dt_e::INT32:
        case jvs::dt_e::UINT32:
            variableData.Value.Boolean = (variableData.Value.UInt32 >> mCIN->GetBitIndex().second) & 1;
            break;
        case jvs::dt_e::INT64:
        case jvs::dt_e::UINT64:
            variableData.Value.Boolean = (variableData.Value.UInt64 >> mCIN->GetBitIndex().second) & 1;
            break;
        default:
            break;
        }

        variableData.DataType  = jvs::dt_e::BOOLEAN;
    }

    void Variable::applyNumericScale(var_data_t& variableData)
    {
        const int8_t exponent = static_cast<int8_t>(mCIN->GetNumericScale().second);
        const double denominator = pow(10, exponent);

        switch (variableData.DataType)
        {
        case jvs::dt_e::INT8:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int8) * denominator;
            break;
        
        case jvs::dt_e::UINT8:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt8) * denominator;
            break;
        
        case jvs::dt_e::INT16:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int16) * denominator;
            break;
        
        case jvs::dt_e::UINT16:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt16) * denominator;
            break;
        
        case jvs::dt_e::INT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<float>(variableData.Value.Int32) * denominator;
            break;
        
        case jvs::dt_e::UINT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<float>(variableData.Value.UInt32) * denominator;
            break;
        
        case jvs::dt_e::FLOAT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Float32) * denominator;
            break;
        
        case jvs::dt_e::INT64:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int64) * denominator;
            break;
        
        case jvs::dt_e::UINT64:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt64) * denominator;
            break;
        
        case jvs::dt_e::FLOAT64:
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
        case jvs::dt_e::INT8:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int8) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::UINT8:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt8) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::INT16:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.Int16) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::UINT16:
            variableData.DataType = jvs::dt_e::FLOAT32;
            variableData.Value.Float32 = static_cast<float>(variableData.Value.UInt16) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::INT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int32) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::UINT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt32) + mCIN->GetNumericOffset().second;
            break;
        
        case jvs::dt_e::FLOAT32:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Float32) + mCIN->GetNumericOffset().second;
            break;

        case jvs::dt_e::INT64:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.Int64) + mCIN->GetNumericOffset().second;
            break;

        case jvs::dt_e::UINT64:
            variableData.DataType = jvs::dt_e::FLOAT64;
            variableData.Value.Float64 = static_cast<double>(variableData.Value.UInt64) + mCIN->GetNumericOffset().second;
            break;
            
        case jvs::dt_e::FLOAT64:
            variableData.Value.Float64 = variableData.Value.Float64 + mCIN->GetNumericOffset().second;
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
        for (auto& dataUnitOrders : mCIN->GetDataUnitOrders().second)
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

                if (dataUnitOrder.DataUnit == jvs::data_unit_e::WORD)
                {
                    arrayOrderedBytes.emplace_back(vectorFlattened[startByteIndex]);
                    arrayOrderedBytes.emplace_back(vectorFlattened[finishByteIndex]);
                }
                else if (dataUnitOrder.DataUnit == jvs::data_unit_e::BYTE)
                {
                    if (dataUnitOrder.ByteOrder == jvs::byte_order_e::HIGHER)
                    {
                        arrayOrderedBytes.emplace_back(vectorFlattened[startByteIndex]);
                    }
                    else
                    {
                        arrayOrderedBytes.emplace_back(vectorFlattened[finishByteIndex]);
                    }
                }
            }

            casted_data_t castedData;
            castByteVector(mCIN->GetDataTypes().second[dataTypeIndex], arrayOrderedBytes, &castedData);
            outputCastedData->emplace_back(castedData);
            ++dataTypeIndex;
        }
    }

    void Variable::castWithoutDataUnitOrder(const std::vector<poll_data_t>& polledData, casted_data_t* outputCastedData)
    {
        ASSERT((outputCastedData != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        
        std::vector<uint8_t> vectorFlattened;
        flattenToByteArray(polledData, &vectorFlattened);
        castByteVector(mCIN->GetDataTypes().second.front(), vectorFlattened, outputCastedData);

        if ((mCIN->GetNodeArea().first == Status::Code::GOOD) && 
            (outputCastedData->ValueType == jvs::dt_e::STRING))
        {
            for (size_t i = 0; i < outputCastedData->Value.String.Length - 1; i += 2)
            {
                std::swap(outputCastedData->Value.String.Data[i], outputCastedData->Value.String.Data[i + 1]);
            }
        }
    }

    bool Variable::isEventOccured(var_data_t& variableData)
    {
        if (mCIN->GetAttributeEvent().second == false)
        {
            // LOG_INFO(logger,"HasAttributeEvent IS FALSE");
            return false;
        }

        if (mDataBuffer.size() == 0)
        {
            // 이벤트 데이터 초기값 전송을 위한 변수입니다. 
            if (mInitEvent == true)
            {   
                mInitEvent = false;
                return true;
            }
            
            LOG_INFO(logger,"mDataBuffer IS 0");
            return false;
        }

        const var_data_t lastestHistory = mDataBuffer.back();

        if (variableData.StatusCode != Status::Code::GOOD)
        {
            if (lastestHistory.StatusCode != variableData.StatusCode)
            {
                return true;
            }
            else
            {
                
                LOG_DEBUG(logger, "variableData.DataType : %d",variableData.DataType);
                return false;
            }
        }

        switch (variableData.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            return lastestHistory.Value.Boolean != variableData.Value.Boolean; 
        case jvs::dt_e::INT8:
            return lastestHistory.Value.Int8 != variableData.Value.Int8;
        case jvs::dt_e::UINT8:
            return lastestHistory.Value.UInt8 != variableData.Value.UInt8;
        case jvs::dt_e::INT16:
            return lastestHistory.Value.Int16 != variableData.Value.Int16;
        case jvs::dt_e::UINT16:
            return lastestHistory.Value.UInt16 != variableData.Value.UInt16;
        case jvs::dt_e::INT32:
            return lastestHistory.Value.Int32 != variableData.Value.Int32;
        case jvs::dt_e::UINT32:
            return lastestHistory.Value.UInt32 != variableData.Value.UInt32;
        case jvs::dt_e::FLOAT32:
            return lastestHistory.Value.Float32 != variableData.Value.Float32;
        case jvs::dt_e::INT64:
            return lastestHistory.Value.Int64 != variableData.Value.Int64;
        case jvs::dt_e::UINT64:
            return lastestHistory.Value.UInt64 != variableData.Value.UInt64;
        case jvs::dt_e::FLOAT64:
            return lastestHistory.Value.Float64 != variableData.Value.Float64;
        case jvs::dt_e::STRING:
            return static_cast<bool>(strcmp(lastestHistory.Value.String.Data, variableData.Value.String.Data));
        case jvs::dt_e::ARRAY:
        {
            LOG_DEBUG(logger, "ARRAY 입니다");
            if (lastestHistory.ArrayValue.size() != variableData.ArrayValue.size())
            {
                LOG_ERROR(logger, "ARRAY SIZE MISMATCH: LASTEST = %u, CURRENT = %u",
                lastestHistory.ArrayValue.size(),
                variableData.ArrayValue.size());
                return false;
            }
            
            switch (variableData.ArrayDataType)
            {
            case jvs::dt_e::BOOLEAN:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Boolean != variableData.ArrayValue.at(i).Boolean) 
                    {
                        return true;
                    }
                }
                return false;
            }   
            case jvs::dt_e::INT8:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Int8 != variableData.ArrayValue.at(i).Int8) 
                    {
                        
                        LOG_DEBUG(logger, "이벤트 발생");
                        return true;
                    }
                }
                
                LOG_DEBUG(logger, "이벤트 발생 X");
                return false;
            }
            case jvs::dt_e::UINT8:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).UInt8 != variableData.ArrayValue.at(i).UInt8) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::INT16:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Int16 != variableData.ArrayValue.at(i).Int16) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::UINT16:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).UInt16 != variableData.ArrayValue.at(i).UInt16) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::INT32:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Int32 != variableData.ArrayValue.at(i).Int32) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::UINT32:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).UInt32 != variableData.ArrayValue.at(i).UInt32) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::FLOAT32:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Float32 != variableData.ArrayValue.at(i).Float32) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::INT64:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Int64 != variableData.ArrayValue.at(i).Int64) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::UINT64:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).UInt64 != variableData.ArrayValue.at(i).UInt64) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::FLOAT64:
            {
                for (size_t i = 0; i < variableData.ArrayValue.size(); ++i) 
                {
                    if (lastestHistory.ArrayValue.at(i).Float64 != variableData.ArrayValue.at(i).Float64) 
                    {
                        return true;
                    }
                }
                return false;
            }
            case jvs::dt_e::STRING:

                LOG_WARNING(logger,"UNSUPPORTED DATA TYPE");
                return false;
            
            default:
                return false;
            }

            
        }
        default:
            return false;
        }
   
        return false;
    }

    string_t Variable::ToMuffinString(const std::string& stdString)
    {
        string_t string;
        string.Length = std::min(stdString.length(), sizeof(string.Data) - 1); // 🔹 최대 길이 제한
        strncpy(string.Data, stdString.c_str(), string.Length);
        string.Data[string.Length] = '\0';  // 🔹 문자열 종료 추가

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
        ASSERT((numberofHistory < MAX_HISTORY_SIZE + 1), "CANNOT RETRIEVE MORE THAN THE MAXIMUM HITORY SIZE");

        std::vector<var_data_t> history;

        auto it = mDataBuffer.end();
        for (size_t i = 0; i < numberofHistory; i++)
        {
            --it;
            history.emplace_back(it.operator*());
        }
        
        return history;
    }

    std::string Variable::Float32ConvertToString(const float& data) const
    {
        auto ret = mCIN->GetPrecision();
        if (ret.first == Status::Code::GOOD)
        {
            char format[10] = {'\0'};
            snprintf(format, sizeof(format), "%%.%df", ret.second);
      
            char buffer[32] = {'\0'};
            snprintf(buffer, sizeof(buffer), format, data);
            return std::string(buffer); 
        }


        if (mCIN->GetNumericScale().first == Status::Code::BAD)
        {
            /**
             * @todo 현재 스케일이 없는 float일때 소수점2자리로 고정시켜 서버로 전송. 추후 수정해야함
             */
            char buffer[32] = {'\0'};     
            snprintf(buffer, sizeof(buffer), "%0.2f", data);
            return std::string(buffer);  

        }
        
        const int8_t exponent = static_cast<int8_t>(mCIN->GetNumericScale().second);
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10] = {'\0'};
        snprintf(format, sizeof(format), "%%.%df", decimalPlaces);

        char buffer[32] = {'\0'};
        snprintf(buffer, sizeof(buffer), format, data);

        return std::string(buffer);
    }

    std::string Variable::Float64ConvertToString(const double& data) const
    {
        auto ret = mCIN->GetPrecision();
        if (ret.first == Status::Code::GOOD)
        {
            char format[10] = {'\0'};
            snprintf(format, sizeof(format), "%%.%df", ret.second);
      
            char buffer[128] = {'\0'};
            snprintf(buffer, sizeof(buffer), format, data);
            return std::string(buffer); 
        }

        if (mCIN->GetNumericScale().first == Status::Code::BAD)
        {
            /**
             * @todo 현재 스케일이 없는 float일때 소수점2자리로 고정시켜 서버로 전송. 추후 수정해야함
             */
            char buffer[128] = {'\0'};     
            snprintf(buffer, sizeof(buffer), "%0.2f", data);
            return std::string(buffer);  
        }
        
        const int8_t exponent = static_cast<int8_t>(mCIN->GetNumericScale().second);
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10] = {'\0'};
        snprintf(format, sizeof(format), "%%.%df", decimalPlaces);

        char buffer[128] = {'\0'};
        snprintf(buffer, sizeof(buffer), format, data);

        return std::string(buffer);
    }

    std::string Variable::FloatConvertToStringForLimitValue(const float& data) const
    {   
        if (mCIN->GetNumericScale().first == Status::Code::BAD)
        {
            if (mDataType == jvs::dt_e::FLOAT32)
            {
                return Float32ConvertToString(data);
            }
            else if (mDataType == jvs::dt_e::FLOAT64)
            {
                return Float64ConvertToString(data);
            }
            else
            {
                uint16_t returnData = (uint16_t)data;
                return std::to_string(returnData);
            }
        }
        
        const int8_t exponent = static_cast<int8_t>(mCIN->GetNumericScale().second);
        
        int decimalPlaces = static_cast<int>(-exponent);
        
        char format[10] = {'\0'};
        snprintf(format, sizeof(format), "%%.%df", decimalPlaces);

        char buffer[128] = {'\0'};
        snprintf(buffer, sizeof(buffer), format, data);

        return std::string(buffer);
    }

    mqtt::topic_e Variable::GetTopic() const
    {
        return mCIN->GetTopic().second;
    }

    std::pair<bool, json_datum_t> Variable::CreateDaqStruct()
    {
        json_datum_t daq;

        strncpy(daq.NodeID, mCIN->GetNodeID().second, sizeof(daq.NodeID));

        if (RetrieveCount() == 0)
        {
            return std::make_pair(false, daq);
        }
        
        var_data_t variableData = RetrieveData();
        daq.SourceTimestamp = variableData.Timestamp;
        daq.Topic = mCIN->GetTopic().second;
        if (Status(variableData.StatusCode) != Status(Status::Code::GOOD))
        {
            LOG_WARNING(logger,"variableData.StatusCode : %s",Status(variableData.StatusCode).c_str());
            return std::make_pair(false, daq);
        }

        switch (variableData.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            daq.Value = variableData.Value.Boolean ? "true" : "false";
            break;
        case jvs::dt_e::FLOAT32 :
            daq.Value = Float32ConvertToString(variableData.Value.Float32);
            break;
        case jvs::dt_e::FLOAT64:
            daq.Value = Float64ConvertToString(variableData.Value.Float64);
            break;
        case jvs::dt_e::INT16:
            daq.Value = std::to_string(variableData.Value.Int16);
            break;
        case jvs::dt_e::INT32:
            daq.Value = std::to_string(variableData.Value.Int32);
            break;
        case jvs::dt_e::INT64:
            daq.Value = std::to_string(variableData.Value.Int64);
            break;
        case jvs::dt_e::INT8 :
            daq.Value = std::to_string(variableData.Value.Int8);
            break;
        case jvs::dt_e::STRING:
            daq.Value = std::string(variableData.Value.String.Data);
            break;
        case jvs::dt_e::UINT16:
            daq.Value = std::to_string(variableData.Value.UInt16);
            break;
        case jvs::dt_e::UINT32:
            daq.Value = std::to_string(variableData.Value.UInt32);
            break;
        case jvs::dt_e::UINT64:
            daq.Value = std::to_string(variableData.Value.UInt64);
            break;
        case jvs::dt_e::UINT8:
            daq.Value = std::to_string(variableData.Value.UInt8);
            break;
    #if defined(MT11)
        case jvs::dt_e::ARRAY:
        {
            daq.isArray = true;
            ArrayConvertToString(variableData.ArrayValue, variableData.ArrayDataType, daq.ArrayValue);
            break;
        }
    #endif
        default:
            break;
        }
        
        return std::make_pair(true, daq);
    }

    bool Variable::ArrayConvertToString(psram::vector<muffin::im::var_value_u> data, jvs::dt_e dataType, std::vector<std::string>& value) const
    {
        value.reserve(data.size());
        for (const auto& datum : data)
        {
            switch (dataType)
            {
            case jvs::dt_e::BOOLEAN:
                value.emplace_back(std::to_string(datum.Boolean));
                break;
            case jvs::dt_e::FLOAT32:
            {
                value.emplace_back(Float32ConvertToString(datum.Float32));  
                break;
            }
            case jvs::dt_e::FLOAT64:
            {
                value.emplace_back(Float64ConvertToString(datum.Float64));  
                break;
            }
            case jvs::dt_e::INT8:
                value.emplace_back(std::to_string(datum.Int8));
                break;
            case jvs::dt_e::INT16:
                value.emplace_back(std::to_string(datum.Int16));
                break;
            case jvs::dt_e::INT32:
                value.emplace_back(std::to_string(datum.Int32));
                break;
            case jvs::dt_e::INT64:
                value.emplace_back(std::to_string(datum.Int64));
                break;
            case jvs::dt_e::UINT8:
                value.emplace_back(std::to_string(datum.UInt8));
                break;
            case jvs::dt_e::UINT16:
                value.emplace_back(std::to_string(datum.UInt16));
                break;
            case jvs::dt_e::UINT32:
                value.emplace_back(std::to_string(datum.UInt32));
                break;
            case jvs::dt_e::UINT64:
                value.emplace_back(std::to_string(datum.UInt64));
                break;
            default:
                // 지원하지 않는 타입은 무시
                break;
            }
        }

        return true;
    }

    std::pair<Status, uint16_t> Variable::StringConvertWordData(std::string& data)
    {
        if (data.empty() == true)
        {
            return std::make_pair(Status(Status::Code::BAD_NO_DATA), 0);
        }
        
        /**
         * @brief 현재 단일 레지스터나 비트만 제어 가능함, 추후 Method 개발시 업데이트 예정입니다.
         * 
         */
        if ((mCIN->GetNumericAddressQuantity().first  == Status::Code::GOOD) &&
            (mCIN->GetNumericAddressQuantity().second != 1))
        {
            LOG_ERROR(logger, "ASCII DATA IS NOT SUPPORTED YET");
            return std::make_pair(Status(Status::Code::BAD_SERVICE_UNSUPPORTED), 0);
        }
        
    
        if (mCIN->GetDataTypes().second.at(0) != jvs::dt_e::STRING)
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

            if (mCIN->GetBitIndex().first == Status::Code::GOOD)
            {
                if (Convert.ToUInt16(data) > 1)
                {
                    LOG_ERROR(logger,"BIT DATA HAS ONLY 1 or 0 VALUE , DATA : %s",data.c_str());
                    return std::make_pair(Status(Status::Code::BAD_NO_DATA_AVAILABLE), 0);
                }
            }

            if (mCIN->GetNumericOffset().first == Status::Code::GOOD)
            {
               floatTemp = Convert.ToFloat(data) - mCIN->GetNumericOffset().second;
              
               if (mCIN->GetNumericScale().first == Status::Code::BAD)
               {
                    //현재 구조에서 offset이 있는데 scale이 없는 경우가 있을지는 모르겠다.
                    return std::make_pair(Status(Status::Code::GOOD), static_cast<uint16_t>(floatTemp));
               }
            }

            if (mCIN->GetNumericScale().first == Status::Code::GOOD)
            {
                const int8_t exponent = static_cast<int8_t>(mCIN->GetNumericScale().second);
                const double denominator = pow(10, exponent);
                
                if (mCIN->GetNumericOffset().first == Status::Code::GOOD)
                {
                    floatTemp = (floatTemp / denominator);
                    uint16_t result = static_cast<uint16_t>(std::ceil(floatTemp));
                    return std::make_pair(Status(Status::Code::GOOD), result);
                }
                else
                {
                    float value = Convert.ToFloat(data) / denominator;
                    uint16_t result = static_cast<uint16_t>(std::round(value));
                    return std::make_pair(Status(Status::Code::GOOD), result);
                }
            }

            LOG_DEBUG(logger, "Raw data : %s, Convert Modbus data : %u" , data.c_str(), Convert.ToUInt16(data));
            return std::make_pair(Status(Status::Code::GOOD), Convert.ToUInt16(data));
        }
        else
        {
            LOG_ERROR(logger, "ASCII DATA IS NOT SUPPORTED YET");
            return std::make_pair(Status(Status::Code::BAD_SERVICE_UNSUPPORTED), 0);
        }
    }



    uint32_t Variable::mSamplingIntervalInMillis = 1000;
}}