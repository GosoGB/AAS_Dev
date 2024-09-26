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

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"



namespace muffin { namespace im {

    class Variable
    {
    public:
        Variable(const data_type_e dataType);
        virtual ~Variable();
    public:
        Status UpdateData(const var_data_t& data);
    private:
        const data_type_e mDataType;
        std::deque<var_data_t> mDataBuffer;
        static uint32_t mSamplingIntervalInMillis;
        const uint8_t mMaxHistorySize = 2;
    };
}}