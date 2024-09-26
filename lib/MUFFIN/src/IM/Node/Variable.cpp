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




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Variable.h"



namespace muffin { namespace im {

    uint32_t Variable::mSamplingIntervalInMillis = 1000;


    Variable::Variable(const data_type_e dataType)
        : mDataType(dataType)
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }

    Variable::~Variable()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status Variable::UpdateData(const var_data_t& data)
    {
        if (mDataBuffer.size() == mMaxHistorySize)
        {
            mDataBuffer.pop_front();
        }
        
        if (mDataType != data_type_e::STRING)
        {
            mDataBuffer.push_back(data);
        }
        else
        {
            ;
        }

        return Status(Status::Code::BAD);
    }
}}