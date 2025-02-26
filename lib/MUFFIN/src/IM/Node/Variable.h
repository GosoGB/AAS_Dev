/**
 * @file Variable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 선언합니다.
 * 
 * @date 2025-02-26
 * @version 1.2.13
 * 
 * @todo 현재는 모든 데이터 수집 주기가 동일하기 때문에 샘플링 인터벌 변수를
 *       static 키워드를 사용해 선언하였습니다. 다만 향후에 노드 별로 수집
 *       주기가 달라진다면 static 키워드를 제거해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <deque>
#include <sys/_stdint.h>
#include <vector>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"
#include "JARVIS/Config/Information/Node.h"
#include "Protocol/Modbus/Include/TypeDefinitions.h"
#include "DataFormat/JSON/JSON.h"


namespace muffin { namespace im {

    class Variable
    {
    public:
        Variable(const std::string& nodeID, const std::string& UID);
        virtual ~Variable() {}
    public:
        void Init(const jvs::config::Node* cin);
    public:
        void Update(const std::vector<poll_data_t>& polledData);
        std::pair<Status,uint16_t> ConvertModbusData(std::string& data);
    private:
        void implUpdate(const std::vector<poll_data_t>& polledData, var_data_t* variableData);
        void removeOldestHistory();
        void flattenToByteArray(const std::vector<poll_data_t>& polledData, std::vector<uint8_t>* outputFlattenVector);
        void castByteVector(const jvs::dt_e dataType, std::vector<uint8_t>& vectorBytes, casted_data_t* castedData);
        void applyBitIndex(var_data_t& variableData);
        void applyNumericScale(var_data_t& variableData);
        void applyNumericOffset(var_data_t& variableData);
        bool isEventOccured(var_data_t& variableData);
        string_t ToMuffinString(const std::string& stdString);
        void logData(const var_data_t& data);
        std::string Float32ConvertToString(const float& data) const;
        std::string Float64ConvertToString(const double& data) const;

    public:
        std::string FloatConvertToStringForLimitValue(const float& data) const;
        
    public:
        size_t RetrieveCount() const;
        var_data_t RetrieveData() const;
        std::vector<var_data_t> RetrieveHistory(const size_t numberOfHistory) const;
    public:
        jvs::addr_u GetAddress() const;
        uint8_t GetQuantity() const;
        uint16_t GetBitIndex() const;
        jvs::mb_area_e GetModbusArea() const;
        std::pair<bool, jvs::scl_e> GetNumericScale() const { return mNumericScale; }
        bool GetHasAttributeEvent() const { return mHasAttributeEvent; }
        std::pair<bool, daq_struct_t> CreateDaqStruct();
    private:
        void castWithDataUnitOrder(const std::vector<poll_data_t>& polledData, std::vector<casted_data_t>* outputCastedData);
        void castWithoutDataUnitOrder(const std::vector<poll_data_t>& polledData, casted_data_t* outputCastedData);

    // 원격제어시 bit 정보를 알기 위해 임시로 만들어둔 매서드입니다. 추후 삭제 예정입니다.
    public:
        std::pair<bool, uint8_t> GetBitindex() const;
    private:
        // virtual void strategySingleDataType() override;
        // void strategySingleDataType();
    private:
        jvs::addr_u mAddress;
        jvs::adtp_e mAddressType;
        bool mHasAttributeEvent;
        std::vector<jvs::dt_e> mVectorDataTypes;
        std::pair<bool, jvs::mb_area_e> mModbusArea;
        std::pair<bool, uint8_t> mBitIndex;
        std::pair<bool, uint8_t> mAddressQuantity;
        std::pair<bool, jvs::scl_e> mNumericScale;
        std::pair<bool, float> mNumericOffset;
        std::pair<bool, std::vector<jvs::DataUnitOrder>> mVectorDataUnitOrders;
        std::pair<bool, std::string> mFormatString;

        // 이벤트 데이터 초기값 전송을 위한 변수입니다. 
        bool mInitEvent = true;
    private:
        const std::string mNodeID;
        const std::string mDeprecableUID;
        jvs::dt_e mDataType;
        std::deque<var_data_t> mDataBuffer;
        static uint32_t mSamplingIntervalInMillis;
        uint8_t mMaxHistorySize = 2;
    
    };
}}