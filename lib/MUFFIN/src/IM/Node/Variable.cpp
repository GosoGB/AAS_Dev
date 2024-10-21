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




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Variable.h"



namespace muffin { namespace im {

    Variable::Variable()
        : mModbusArea(false, jarvis::mb_area_e::COILS)
        , mBitIndex(false, 0)
        , mAddressQuantity(false, 0)
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

    /**
     * @todo jump table로 처리하는 것이 필요한지 고민해보고 필요하면 적용해야 합니다.
     */
    void Variable::Update(const poll_data_t& polledData)
    {
        /**
         * @todo 문자열의 경우 메모리가 부족할 수도 있기 때문에 단일 데이터만
         *       저장될 수 있도록 설계를 바꿔야 할지에 대한 결정이 필요합니다.
         */
        if (mDataBuffer.size() == mMaxHistorySize)
        {
            mDataBuffer.pop_front();
        }

        var_data_t data;
        data.StatusCode     = polledData.StatusCode;
        data.Timestamp      = polledData.Timestamp;
        data.DataType       = mDataType;
        data.HasStatus      = true;
        /**
         * @todo 현재는 NTP 서버와 동기화가 되어야 MUFFIN 초기화가 끝나기
         *       때문에 항상 true일 수밖에 없습니다. 따라서 해당 속성을
         *       없애는 것을 고려해봐야 합니다.
         */
        data.HasTimestamp   = true;

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


    uint32_t Variable::mSamplingIntervalInMillis = 1000;
}}