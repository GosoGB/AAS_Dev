/**
 * @file Variable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 정의합니다.
 * 
 * @date 2024-09-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <stdarg.h>
#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Variable.h"



namespace muffin { namespace im {

    Variable::Variable()
        : mModbusArea(false, jarvis::mb_area_e::COILS)
        , mBitIndex(false, 0)
        , mAddressQuantity(false, 1)
        , mNumericScale(false, jarvis::scl_e::NEGATIVE_1)
        , mNumericOffset(false, 0.0f)
        , mMapMappingRules(false, std::map<std::uint16_t, std::string>())
        , mVectorDataUnitOrders(false, std::vector<jarvis::DataUnitOrder>())
        , mFormatString(false, std::string())
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

    std::string createFormattedString(const char* fmt, std::vector<casted_data_t>& inputVector)
    {
        return "UNSUPPORTED SERVICE: TOO MANY FORMAT SPECIFIERS";
    }

    /**
     * @todo jump table로 처리하는 것이 필요한지 고민해보고 필요하면 적용해야 합니다.
     */
    void Variable::Update(const poll_data_t& polledData)
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


        if (mVectorDataUnitOrders.first == true)
        {// data unit order is configured
            std::vector<poll_data_t> vectorPolledData;
            vectorPolledData.reserve(1);
            vectorPolledData.emplace_back(polledData);

            std::vector<casted_data_t> vectorCastedData;
            castWithDataUnitOrder(vectorPolledData, &vectorCastedData);

            var_data_t variableData;
            variableData.StatusCode     = polledData.StatusCode;
            variableData.Timestamp      = polledData.Timestamp;

            if (mVectorDataUnitOrders.second.size() == 1)
            {
                variableData.DataType  = vectorCastedData[0].ValueType;
                variableData.Value     = vectorCastedData[0].Value;

                if (mVectorDataTypes[0] != jarvis::dt_e::STRING)
                {
                    if (mBitIndex.first == true)
                    {
                        variableData.DataType  = jarvis::dt_e::BOOLEAN;
                        variableData.Value     = vectorCastedData[0].Value;
                        uint8_t bitMask = 1 << mBitIndex.second;
                        variableData.Value.Boolean = (variableData.Value.UInt64 & bitMask) >> mBitIndex.second;
                    }

                    if (mNumericScale.first == true)
                    {
                        /* code */
                    }
                    
                }
            }
            else
            {
                std::string formattedString = createFormattedString(mFormatString.second.c_str(), vectorCastedData);
                LOG_DEBUG(logger, "Formatted string: %s", formattedString.c_str());

                variableData.DataType       = jarvis::dt_e::STRING;
                variableData.Value.String   = ToMuffinString(formattedString);
            }

            variableData.HasValue      = true;
            variableData.HasStatus      = true;
            variableData.HasTimestamp   = true;
        }

        /**
         * @todo 현재는 NTP 서버와 동기화가 되어야 MUFFIN 초기화가 끝나기 때문에 항상 true일 수밖에 없습니다.
         *       따라서 해당 속성이 필요한지 고민한 다음 필요 없다면 제거해야 합니다.
         */
        variableData.HasTimestamp   = true;
        variableData.HasStatus      = true;


        /**
         * @todo ord, scl, ofst 등을 고려해야 합니다.
         */
        if (data.StatusCode != Status::Code::GOOD)
        {
            data.HasValue = false;

            if (mDataBuffer.size() == 0)
            {
                data.IsEventType  = true;
                data.HasNewEvent  = true;
                goto EMPLACE_DATA;
            }
            else
            {
                const var_data_t lastestHistory = mDataBuffer.back();
                data.IsEventType  = (lastestHistory.StatusCode != data.StatusCode);
                data.HasNewEvent  = (lastestHistory.StatusCode != data.StatusCode);
                goto EMPLACE_DATA;
            }
        }
        else
        {
            data.HasValue = true;
            if (mDataType == jarvis::dt_e::STRING)
            {
                processStringData(polledData, &data);
            }
            else
            {
                processNumericData(polledData, &data);
            }
        }

    EMPLACE_DATA:
        try
        {
            mDataBuffer.emplace_back(data);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE DATA: %s", e.what());
        }
    }
    
    void Variable::Update(const std::vector<poll_data_t>& polledData)
    {
        /**
         * @todo 문자열의 경우 메모리가 부족할 수도 있기 때문에 단일 데이터만
         *       저장될 수 있도록 설계를 바꿔야 할지에 대한 결정이 필요합니다.
         */
        if (mDataBuffer.size() == mMaxHistorySize)
        {
            mDataBuffer.pop_front();
        }

        /**
         * @todo 현재는 시간 상의 이유로 Modbus 프로토콜에 워드 데이터만 처리할 수 있도록
         *       작업해두었습니다. 구조도 깔끔하지 않아서 향후에 개선하는 작업이 필요합니다.
         */
        var_data_t data;
        data.Timestamp   = polledData[0].Timestamp;
        for (auto& polledDatum : polledData)
        {
            if (polledDatum.StatusCode != Status::Code::GOOD)
            {
                data.StatusCode = polledDatum.StatusCode;
            }
            
            if (data.Timestamp != polledDatum.Timestamp)
            {
                data.StatusCode = Status::Code::BAD_INVALID_TIMESTAMP;
            }
        }
        data.HasStatus   = true;


        if ((mVectorDataUnitOrders.first == true))
        {
            ASSERT((mVectorDataUnitOrders.second.size() == 0), "LENGTH OF DATA UNIT ORDERS CANNOT BE 0 WHEN IT'S ENALBED");

            if (mVectorDataUnitOrders.second.size() == 1)
            {
                std::vector<casted_data_t> castedData;
                castWithDataUnitOrder(polledData, &castedData);
                
                if (castedData.size() == 1)
                {
                    if (castedData[0].ValueType == jarvis::dt_e::STRING)
                    {
                        data.DataType = jarvis::dt_e::STRING;
                        data.Value = castedData[0].Value;
                    }
                }
                
                if (mBitIndex.first == true)
                {
                    // apply bit index;
                        // castedData[0].Value.Boolean & (0x01 << mBitIndex.second);
                        // mDataBuffer.emplace_back()
                }

                if (mMapMappingRules.first == true)
                {
                    // apply mapping rules;
                }

                if (mNumericScale.first == true)
                {
                    // apply numeric scale;
                }

                if (mNumericOffset.first == true)
                {
                    // apply numeric offset;
                }
            }
            else
            {
                // apply format string;
            }
        }
        else
        {
            castWithoutDataUnitOrder(polledData);
            // if bit index true:
            //     apply bit index
            // if mapping rules ture:
            //     apply mapping rules;
            // if numeric scale ture:
            //     apply numeric scale;
            // if numeric offset ture:
            //     apply numeric offset;
        }


        if (mVectorDataUnitOrders.first == true && mVectorDataUnitOrders.second.size() == 1)
        {
            ASSERT((polledData.size() == 1), "ADDRESS QUANTITY AND THE POLLED QUANTITY MUST BE IDENTICAL");
            ASSERT((mVectorDataTypes.size() == 1), "THERE MUST BE ONE DATA TYPE AND THE POLLED QUANTITY MUST BE IDENTICAL");

        }


        /**
         * @todo 현재는 NTP 서버와 동기화가 되어야 MUFFIN 초기화가 끝나기
         *       때문에 항상 true일 수밖에 없습니다. 따라서 해당 속성을
         *       없애는 것을 고려해봐야 합니다.
         */
        // data.HasTimestamp   = true;
    }
    
    void Variable::castWithDataUnitOrder(const std::vector<poll_data_t>& polledData, std::vector<casted_data_t>* outputCastedData)
    {
        ASSERT((outputCastedData != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        ASSERT((outputCastedData->empty() == true), "OUTPUT PARAMETER MUST BE AN EMPTY ARRAY");

        /**
         * @brief 현재는 기계에서 수집한 데이터의 타입이 16비트일 때까지만 구현되어 있습니다.
         *        향후 다른 프로토콜이 필요하므로 나머지 데이터 타입의 처리를 구현해야 합니다.
         */
        uint8_t idxDataTypeVector = 0;
        for (auto& dataUnitOrders : mVectorDataUnitOrders.second)
        {
            const bool isDataTypeString = mVectorDataTypes[idxDataTypeVector] == jarvis::dt_e::STRING;
            ++idxDataTypeVector;

            if (isDataTypeString == false)
            {
                const size_t totalSizeOfBytes = dataUnitOrders.RetrieveTotalSize();
                uint8_t arrayBytesPolled[totalSizeOfBytes] = { 0 };
                uint8_t indexPolled = 0;

                for (auto& datum : polledData)
                {
                    switch (datum.ValueType)
                    {
                    case jarvis::dt_e::INT8:
                    case jarvis::dt_e::UINT8:
                        arrayBytesPolled[indexPolled] = datum.Value.UInt8;
                        ++indexPolled;
                        break;
                    case jarvis::dt_e::INT16:
                    case jarvis::dt_e::UINT16:
                        arrayBytesPolled[indexPolled] = static_cast<uint8_t>(((datum.Value.UInt16 >> 8) & 0xFF));
                        ++indexPolled;
                        arrayBytesPolled[indexPolled] = static_cast<uint8_t>((datum.Value.UInt16 & 0xFF));
                        ++indexPolled;
                        break;
                    /**
                     * @todo 32bit, 64bit인 경우를 구현해야 합니다.
                     */
                    default:
                        break;
                    }
                }
                ASSERT((totalSizeOfBytes == indexPolled), "TOTAL DATA UNIT ORDER SIZE AND THE POLLED DATA SIZE MUST IDENTICAL");


                uint8_t arrayBytesCasted[totalSizeOfBytes] = { 0 };
                uint8_t indexCasted = 0;
                for (auto& dataUnitOrder : dataUnitOrders)
                {
                    /**
                     * @todo 32bit, 64bit인 경우를 구현해야 합니다.
                     */
                    const uint8_t startIndex  = 2 * dataUnitOrder.Index;
                    const uint8_t finishIndex = startIndex + 1;

                    if (dataUnitOrder.DataUnit == jarvis::data_unit_e::WORD)
                    {
                        arrayBytesCasted[indexCasted] = arrayBytesPolled[startIndex];
                        ++indexCasted;
                        arrayBytesCasted[indexCasted] = arrayBytesPolled[finishIndex];
                        ++indexCasted;
                    }
                    else if (dataUnitOrder.DataUnit == jarvis::data_unit_e::BYTE)
                    {
                        if (dataUnitOrder.ByteOrder == jarvis::byte_order_e::HIGHER)
                        {
                            arrayBytesCasted[indexCasted] = arrayBytesPolled[startIndex];
                        }
                        else
                        {
                            arrayBytesCasted[indexCasted] = arrayBytesPolled[finishIndex];
                        }
                        ++indexCasted;
                    }
                }

                casted_data_t castedData;
                switch (mVectorDataTypes[idxDataTypeVector])
                {
                case jarvis::dt_e::INT8:
                    castedData.ValueType = jarvis::dt_e::INT8;
                    castedData.Value.UInt8 = *reinterpret_cast<int8_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::UINT8:
                    castedData.ValueType = jarvis::dt_e::UINT8;
                    castedData.Value.UInt8 = *reinterpret_cast<uint8_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::INT16:
                    castedData.ValueType = jarvis::dt_e::INT16;
                    castedData.Value.UInt8 = *reinterpret_cast<int16_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::UINT16:
                    castedData.ValueType = jarvis::dt_e::UINT16;
                    castedData.Value.UInt8 = *reinterpret_cast<uint16_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::INT32:
                    castedData.ValueType = jarvis::dt_e::INT32;
                    castedData.Value.UInt8 = *reinterpret_cast<int32_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::UINT32:
                    castedData.ValueType = jarvis::dt_e::UINT32;
                    castedData.Value.UInt8 = *reinterpret_cast<uint32_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::INT64:
                    castedData.ValueType = jarvis::dt_e::INT64;
                    castedData.Value.UInt8 = *reinterpret_cast<int64_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::UINT64:
                    castedData.ValueType = jarvis::dt_e::UINT64;
                    castedData.Value.UInt8 = *reinterpret_cast<uint64_t*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::FLOAT32:
                    castedData.ValueType = jarvis::dt_e::FLOAT32;
                    castedData.Value.UInt8 = *reinterpret_cast<float*>(arrayBytesCasted);
                    break;
                case jarvis::dt_e::FLOAT64:
                    castedData.ValueType = jarvis::dt_e::FLOAT64;
                    castedData.Value.UInt8 = *reinterpret_cast<double*>(arrayBytesCasted);
                    break;
                default:
                    break;
                }

                outputCastedData->emplace_back(castedData);
            }
            else
            {
                /**
                 * @todo im::string_t 개체는 동적할당이기 때문에 소멸자를 나중에 호출시켜줘야 합니다.
                 */
                std::string castedString;
                for (auto& datum : polledData)
                {
                    switch (datum.ValueType)
                    {
                    case jarvis::dt_e::INT8:
                    case jarvis::dt_e::UINT8:
                        castedString += static_cast<char>(datum.Value.UInt8);
                        break;
                    case jarvis::dt_e::INT16:
                    case jarvis::dt_e::UINT16:
                        castedString += static_cast<char>(((datum.Value.UInt16 >> 8) & 0xFF));
                        castedString += static_cast<char>(((datum.Value.UInt16 & 0xFF)));
                        break;
                    /**
                     * @todo 32bit, 64bit인 경우를 구현해야 합니다.
                     */
                    default:
                        break;
                    }
                }

                casted_data_t castedData;
                castedData.ValueType = jarvis::dt_e::STRING;
                castedData.Value.String = ToMuffinString(castedString);

                outputCastedData->emplace_back(castedData);
            }
        }
    }

    void Variable::castWithoutDataUnitOrder(const std::vector<poll_data_t>& polledData, casted_data_t* outputCastedData)
    {
        ASSERT((outputCastedData != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        const bool isDataTypeString = mVectorDataTypes[0] == jarvis::dt_e::STRING;
        ASSERT((mVectorDataTypes.size() == 1), "THE SIZE OF DATA TYPE ARRAY MUST BE 1 WHEN DATA UNIT ORDER IS DISABLED");

        if (isDataTypeString == false)
        {
            std::vector<uint8_t> vectorBytesPolled;

            for (auto& datum : polledData)
            {
                switch (datum.ValueType)
                {
                case jarvis::dt_e::INT8:
                case jarvis::dt_e::UINT8:
                    vectorBytesPolled.emplace_back(datum.Value.UInt8);
                    break;
                case jarvis::dt_e::INT16:
                case jarvis::dt_e::UINT16:
                    vectorBytesPolled.emplace_back(static_cast<uint8_t>(((datum.Value.UInt16 >> 8) & 0xFF)));
                    vectorBytesPolled.emplace_back(static_cast<uint8_t>(((datum.Value.UInt16 & 0xFF))));
                    break;
                /**
                 * @todo 32bit, 64bit인 경우를 구현해야 합니다.
                 */
                default:    
                    ASSERT(false, "UNDEFINED BEHAVIOUR");
                    break;
                }
            }


            switch (mVectorDataTypes[0])
            {
            case jarvis::dt_e::INT8:
                outputCastedData->ValueType = jarvis::dt_e::INT8;
                outputCastedData->Value.UInt8 = *reinterpret_cast<int8_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::UINT8:
                outputCastedData->ValueType = jarvis::dt_e::UINT8;
                outputCastedData->Value.UInt8 = *reinterpret_cast<uint8_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::INT16:
                outputCastedData->ValueType = jarvis::dt_e::INT16;
                outputCastedData->Value.UInt8 = *reinterpret_cast<int16_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::UINT16:
                outputCastedData->ValueType = jarvis::dt_e::UINT16;
                outputCastedData->Value.UInt8 = *reinterpret_cast<uint16_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::INT32:
                outputCastedData->ValueType = jarvis::dt_e::INT32;
                outputCastedData->Value.UInt8 = *reinterpret_cast<int32_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::UINT32:
                outputCastedData->ValueType = jarvis::dt_e::UINT32;
                outputCastedData->Value.UInt8 = *reinterpret_cast<uint32_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::INT64:
                outputCastedData->ValueType = jarvis::dt_e::INT64;
                outputCastedData->Value.UInt8 = *reinterpret_cast<int64_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::UINT64:
                outputCastedData->ValueType = jarvis::dt_e::UINT64;
                outputCastedData->Value.UInt8 = *reinterpret_cast<uint64_t*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::FLOAT32:
                outputCastedData->ValueType = jarvis::dt_e::FLOAT32;
                outputCastedData->Value.UInt8 = *reinterpret_cast<float*>(vectorBytesPolled.data());
                break;
            case jarvis::dt_e::FLOAT64:
                outputCastedData->ValueType = jarvis::dt_e::FLOAT64;
                outputCastedData->Value.UInt8 = *reinterpret_cast<double*>(vectorBytesPolled.data());
                break;
            default:
                ASSERT(false, "UNDEFINED BEHAVIOUR");
                break;
            }

            return;
        }
        else
        {
            /**
             * @todo im::string_t 개체는 동적할당이기 때문에 소멸자를 나중에 호출시켜줘야 합니다.
             */
            std::string castedData;
            for (auto& datum : polledData)
            {
                switch (datum.ValueType)
                {
                case jarvis::dt_e::INT8:
                case jarvis::dt_e::UINT8:
                    castedData += static_cast<char>(datum.Value.UInt8);
                    break;
                case jarvis::dt_e::INT16:
                case jarvis::dt_e::UINT16:
                    castedData += static_cast<char>(((datum.Value.UInt16 >> 8) & 0xFF));
                    castedData += static_cast<char>(((datum.Value.UInt16 & 0xFF)));
                    break;
                /**
                 * @todo 32bit, 64bit인 경우를 구현해야 합니다.
                 */
                default:
                    break;
                }
            }

            outputCastedData->ValueType = jarvis::dt_e::STRING;
            outputCastedData->Value.String = ToMuffinString(castedData);
        }
    }

    void Variable::strategySingleDataType()
    {
        ;
    }
    
    void Variable::processStringData(const poll_data_t& polledData, var_data_t* outputData)
    {
        if (mMapMappingRules.first == true)
        {
            std::string stdString;

            switch (polledData.ValueType)
            {
            case jarvis::dt_e::BOOLEAN:
                stdString = mMapMappingRules.second[polledData.Value.Boolean];
                break;
            case jarvis::dt_e::UINT16:
                stdString = mMapMappingRules.second[polledData.Value.UInt16];
                break;
            default:
                ASSERT(false, "UNSUPPORTED DATA TYPE");
                break;
            }

            outputData->Value.String = ToMuffinString(stdString);

            if (mDataBuffer.size() == 0)
            {
                outputData->IsEventType  = true;
                outputData->HasNewEvent  = true;
            }
            else
            {
                const var_data_t lastestHistory = mDataBuffer.back();
                outputData->IsEventType  = (strcmp(lastestHistory.Value.String.Data, outputData->Value.String.Data) != 0);
                outputData->HasNewEvent  = (strcmp(lastestHistory.Value.String.Data, outputData->Value.String.Data) != 0);
            }
        }
        else
        {
            ASSERT(false, "IMPLEMENTATION ERROR: FORMAT STRING IS NOT IMPLEMENTED");
        }
    }
    
    void Variable::processNumericData(const poll_data_t& polledData, var_data_t* outputData)
    {
        ;
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

    uint32_t Variable::mSamplingIntervalInMillis = 1000;
}}