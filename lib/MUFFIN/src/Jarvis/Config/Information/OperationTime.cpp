/**
 * @file OperationTime.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동 시간 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "OperationTime.h"



namespace muffin { namespace jarvis { namespace config {

    OperationTime::OperationTime()
        : Base(cfg_key_e::OPERATION_TIME)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    OperationTime::~OperationTime()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    OperationTime& OperationTime::operator=(const OperationTime& obj)
    {
        if (this != &obj)
        {
            mNodeID     =  obj.mNodeID;       
            mType       =  obj.mType;
            mCriterion  =  obj.mCriterion;
            mOperater   =  obj.mOperater;
        }
        
        return *this;
    }

    bool OperationTime::operator==(const OperationTime& obj) const
    {
       return (
            mNodeID     ==  obj.mNodeID      &&       
            mType       ==  obj.mType        &&
            mCriterion  ==  obj.mCriterion   &&
            mOperater   ==  obj.mOperater
        );
    }

    bool OperationTime::operator!=(const OperationTime& obj) const
    {
        return !(*this == obj);
    }

    void OperationTime::SetNodeID(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeID = nodeID;
        mIsNodeIdSet = true;
    }

    void OperationTime::SetType(const op_time_type_e type)
    {
        mType = type;
        mIsTypeSet = true;
    }

    void OperationTime::SetCriterion(const int32_t criterion)
    {
        mCriterion = criterion;
        mIsCriterionSet = true;
    }

    void OperationTime::SetOperator(const cmp_op_e operater)
    {
        mOperater = operater;
        mIsOperatorSet = true;
    }

    std::pair<Status, std::string> OperationTime::GetNodeID() const
    {
        if (mIsNodeIdSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeID);
        }
    }

    std::pair<Status, op_time_type_e> OperationTime::GetType() const
    {
        if (mIsTypeSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mType);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mType);
        }
    }

    std::pair<Status, int32_t> OperationTime::GetCriterion() const
    {
        if (mIsCriterionSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mCriterion);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mCriterion);
        }
    }

    std::pair<Status, cmp_op_e> OperationTime::GetOperator() const
    {
        if (mIsOperatorSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mOperater);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mOperater);
        }
    }
}}}


