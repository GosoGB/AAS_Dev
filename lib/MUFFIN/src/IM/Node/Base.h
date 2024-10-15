/**
 * @file Base.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Base Node 클래스를 선언합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */





/* For future MUFFIN code base
#pragma once

#include "Common/Status.h"
#include "Include/NodeID.h"
#include "Include/TypeDefinitions.h"



namespace muffin { namespace im {
    class Base
    {
    protected:
        Base(const NodeID& nodeID, const class_type_e& classType, const qualified_name_t& browseName);
        virtual ~Base();
    protected:
        void setDisplayName(const local_txt_t& displayName);
        void addReferenceKind(const ref_kind_t& referenceKind);
        Status addReferenceTarget(const ref_target_t& referenceTarget, const int32_t targetNameHash);
    protected:
        node_ptr_u createNodePointerUnion(const NodeID& nodeID);
    protected:
        Status copyDataTypes(const void* source, void* sink, const DataType* type);
        Status copyNodePointer(node_ptr_u& source, node_ptr_u* sink);
        Status copyNodeID(const NodeID& source, NodeID* sink);
    protected:
        const NodeID mNodeID;
        const class_type_e mClassType;
        const qualified_name_t mBrowseName;
        local_txt_t mDisplayName;
        // local_txt_t mDescription;
        // uint32_t mWriteMask;
        uint8_t mReferenceSize;
        ref_kind_t* mReferences;
        // void* mContext;
        // bool mIsConstructed;
        // UA_MonitoredItem* monitoredItems;
    private:
        // (*copyFunc)(const void* source, void* sink, const )
    private:
        static constexpr uint8_t NODE_POINTER_MASK                 = 0x02;
        static constexpr uint8_t NODE_POINTER_TAG_IMMEDIATE        = 0x00;
        static constexpr uint8_t NODE_POINTER_TAG_NODE_ID          = 0x01;
        static constexpr uint8_t NODE_POINTER_TAG_NODE             = 0x02;
    };
}}
*/