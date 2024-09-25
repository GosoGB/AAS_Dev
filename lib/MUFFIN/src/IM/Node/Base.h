/**
 * @file Base.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Base Node 클래스를 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "include/NodeID.h"
#include "include/TypeDefinitions.h"


namespace muffin { namespace im {

    class Base
    {
    public:
        explicit Base(const NodeID& nodeID);
        virtual ~Base();
    public:
        ;
    protected:
        const NodeID mNodeID;
        class_type_e mClassType;
        qualified_name_t mBrowseName;
        local_txt_t mDisplayName;
        // local_txt_t mDescription;
        // uint32_t mWriteMask;
        uint8_t mReferenceSize;
        ref_kind_t* mReferences;
        // void* mContext;
        // bool mIsConstructed;
        // UA_MonitoredItem* monitoredItems;
    };
}}