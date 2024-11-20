/**
 * @file OperationTime.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동 시간 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {

    class OperationTime : public Base
    {
    public:
        OperationTime();
        virtual ~OperationTime() override;
    public:
        OperationTime& operator=(const OperationTime& obj);
        bool operator==(const OperationTime& obj) const;
        bool operator!=(const OperationTime& obj) const;
    public:
        void SetNodeID(const std::string& nodeID);
        void SetType(const op_time_type_e type);
        void SetCriterion(const int32_t criterion);
        void SetOperator(const cmp_op_e operater);
    public:
        std::pair<Status, std::string> GetNodeID() const;
        std::pair<Status, op_time_type_e> GetType() const;
        std::pair<Status, int32_t> GetCriterion() const;
        std::pair<Status, cmp_op_e> GetOperator() const;
    private:
        bool mIsNodeIdSet      = false;
        bool mIsTypeSet        = false;
        bool mIsCriterionSet   = false;
        bool mIsOperatorSet    = false;
    private:
        std::string mNodeID;
        op_time_type_e mType;
        int32_t mCriterion;
        cmp_op_e mOperater;
    };
}}}
