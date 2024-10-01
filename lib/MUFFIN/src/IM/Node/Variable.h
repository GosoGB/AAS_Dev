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
#include "Protocol/Modbus/Include/TypeDefinitions.h"



namespace muffin { namespace im {

    class Variable
    {
    public:
        /* @todo jarvis config 인스턴스로 대체해야 함 */
        explicit Variable(const data_type_e dataType);
        virtual ~Variable();
    public:
        Status Update(const bool value);
        Status Update(const uint8_t size, const uint16_t value[]);
    public:
        Status UpdateData(const var_data_t& data);
        var_data_t RetrieveData() const;
        std::vector<var_data_t> RetrieveHistory(const size_t numberOfHistory) const;
    public:
        /**
         * @todo node cin 속성에 따라 데이터 처리하는 함수 작성해야 합니다.
         */
        void SetAddress(const uint16_t address) { mAddress = address; }
        void SetQuantity(const uint16_t quantity) { mQuantity = quantity; }
        void SetModbusArea(const modbus::area_e area) { mModbusArea = area; }
    public:
        uint16_t GetAddress() const { return mAddress; }
        uint16_t GetQuantity() const { return mQuantity; }
        uint16_t GetBitIndex() const { return mBitIndex; }
        modbus::area_e GetModbusArea() const { return mModbusArea; }
    private:
        /**
         * @todo node cin 을 이동 생성자로 가지고 오는 게 좋습니다.
         */
        uint16_t mAddress;
        uint16_t mQuantity;
        uint16_t mBitIndex;
        modbus::area_e mModbusArea;
    private:
        const data_type_e mDataType;
        std::deque<var_data_t> mDataBuffer;
        static uint32_t mSamplingIntervalInMillis;
        const uint8_t mMaxHistorySize = 5;
    };
}}