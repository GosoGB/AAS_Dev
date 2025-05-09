/**
 * @file Variable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 수집한 데이터를 표현하는 Variable Node 클래스를 선언합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @todo 현재는 모든 데이터 수집 주기가 동일하기 때문에 샘플링 인터벌 변수를
 *       static 키워드를 사용해 선언하였습니다. 다만 향후에 노드 별로 수집
 *       주기가 달라진다면 static 키워드를 제거해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

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
        Variable(const jvs::config::Node* cin);
        ~Variable() {}
    public:
        const char* GetNodeID() const;
        jvs::addr_u GetAddress() const;
        uint8_t GetQuantity() const;
        int16_t GetBitIndex() const;
        jvs::node_area_e GetNodeArea() const;

    public:
        void Update(const std::vector<poll_data_t>& polledData);
        void UpdateError();
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
        std::string Float32ConvertToString(const float& data) const;
        std::string Float64ConvertToString(const double& data) const;

    public:
        std::string FloatConvertToStringForLimitValue(const float& data) const;
        
    public:
        size_t RetrieveCount() const;
        var_data_t RetrieveData() const;
        std::vector<var_data_t> RetrieveHistory(const size_t numberOfHistory) const;

    public:
        /* Convert Remote Control Request To Modbus Format */
        std::pair<Status, uint16_t> ConvertModbusData(std::string& data);
    public:
        std::pair<bool, json_datum_t> CreateDaqStruct();
    private:
        void castWithDataUnitOrder(const std::vector<poll_data_t>& polledData, std::vector<casted_data_t>* outputCastedData);
        void castWithoutDataUnitOrder(const std::vector<poll_data_t>& polledData, casted_data_t* outputCastedData);

    private:
        // virtual void strategySingleDataType() override;
        // void strategySingleDataType();
    
    public:
        bool mHasNewEvent = false;
    private:
        // 이벤트 데이터 초기값 전송을 위한 변수입니다. 
        bool mInitEvent = true;
        jvs::dt_e mDataType;
        std::vector<var_data_t> mDataBuffer;
        const jvs::config::Node* const mCIN;
        static const uint8_t MAX_HISTORY_SIZE = 2;
        static uint32_t mSamplingIntervalInMillis;
    };
}}