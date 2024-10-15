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
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Variable::~Variable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    /**
     * @todo jump table로 처리하는 것이 필요한지 고민해보고 필요하면 적용해야 합니다.
     */
    Status Variable::UpdateData(const var_data_t& data)
    {
        if (mDataType != data_type_e::STRING)
        {
            if (mDataBuffer.size() == mMaxHistorySize)
            {
                mDataBuffer.pop_front();
            }
            mDataBuffer.emplace_back(data);
            return Status(Status::Code::GOOD);
        }
        else
        {
            ASSERT(false, "NOT SUPPORTED DATA TYPE");
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }
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
}}