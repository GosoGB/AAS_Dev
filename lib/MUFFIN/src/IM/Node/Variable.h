/**
 * @file Variable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 선언합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @todo 현재는 모든 데이터 수집 주기가 동일하기 때문에 샘플링 인터벌 변수를
 *       static 키워드를 사용해 선언하였습니다. 다만 향후에 노드 별로 수집
 *       주기가 달라진다면 static 키워드를 제거해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <deque>
#include <sys/_stdint.h>
#include <vector>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"
#include "Jarvis/Config/Information/Node.h"
#include "Protocol/Modbus/Include/TypeDefinitions.h"



namespace muffin { namespace im {

    class Variable
    {
    public:
        Variable();
        virtual ~Variable();
    public:
        void Init(const jarvis::config::Node* cin);
    public:
        void Update(const poll_data_t& polledData);
        void Update(const std::vector<poll_data_t>& polledData);
    private:
        // void processStatusCode(const poll_data_t& polledData);
        void processStringData(const poll_data_t& polledData, var_data_t* outputData);
        void processNumericData(const poll_data_t& polledData, var_data_t* outputData);
        string_t ToMuffinString(const std::string& stdString);

    public:
        var_data_t RetrieveData() const;
        std::vector<var_data_t> RetrieveHistory(const size_t numberOfHistory) const;
    public:
        jarvis::addr_u GetAddress() const;
        uint8_t GetQuantity() const;
        uint16_t GetBitIndex() const;
        jarvis::mb_area_e GetModbusArea() const;
    private:
        jarvis::adtp_e mAddressType;
        jarvis::addr_u mAddress;
        std::vector<jarvis::dt_e> mVectorDataTypes;
        bool mHasAttributeEvent;
        std::string mDeprecableDisplayName;
        std::string mDeprecableDisplayUnit;
        std::pair<bool, jarvis::mb_area_e> mModbusArea;
        std::pair<bool, uint8_t> mBitIndex;
        std::pair<bool, uint8_t> mAddressQuantity;
        std::pair<bool, jarvis::scl_e> mNumericScale;
        std::pair<bool, float> mNumericOffset;
        std::pair<bool, std::map<std::uint16_t, std::string>> mMapMappingRules;
        std::pair<bool, std::vector<jarvis::DataUnitOrder>> mVectorDataUnitOrders;
        std::pair<bool, std::string> mFormatString;
    private:
        jarvis::dt_e mDataType;
        std::deque<var_data_t> mDataBuffer;
        static uint32_t mSamplingIntervalInMillis;
        uint8_t mMaxHistorySize = 5;
    };
}}