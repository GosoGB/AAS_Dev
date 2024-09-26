/**
 * @file Base.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Base Node 클래스를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Base.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/Helper.h"



namespace muffin { namespace im { 
    
    Base::Base()
    {
    }
    
    Base::~Base()
    {
    }


/* For future MUFFIN code base
    Base::Base(const NodeID& nodeID, const class_type_e& classType, const qualified_name_t& browseName)
        : mNodeID(std::move(nodeID))
        , mClassType(classType)
        , mBrowseName(browseName)
    {
    #if defined(DEBUG)
        if (mNodeID.GetType() == node_id_type_e::NUMERIC)
        {
            LOG_DEBUG(logger, "Constructed at address: %p  [Node Class: %s] [NodeID: %u]",
                this,
                ConvertClassTypeEnum(mClassType),
                mNodeID.GetID().Numeric
            );
        }
        else
        {
            LOG_DEBUG(logger, "Constructed at address: %p  [Node Class: %s] [NodeID: %s]",
                this, 
                ConvertClassTypeEnum(mClassType),
                ConvertString(mNodeID.GetID().String).c_str()
            );
        }
    #endif
    }
    
    Base::~Base()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    void Base::setDisplayName(const local_txt_t& displayName)
    {
        mDisplayName = displayName;
    }

    void Base::addReferenceKind(const ref_kind_t& referenceKind)
    {
        ;
    }

    Status Base::addReferenceTarget(const ref_target_t& referenceTarget, const int32_t targetNameHash)
    {
        ref_target_t* newRefTarget = (ref_target_t*)realloc(
            mReferences->Targets, 
            sizeof(ref_target_t) * (mReferences->TargetSize + 1)
        );

        if (newRefTarget == nullptr)
        {
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        mReferences->Targets = newRefTarget;

        // addReferenceTarget

        // Status ret = UA_NodePointer_copy(
        //                         targetId,
        //                         &rk->targets.array[rk->targetsSize].targetId
        //                     );

        // rk->targets.array[rk->targetsSize].targetNameHash = targetNameHash;
        // if(retval != UA_STATUSCODE_GOOD) {
        //     if(rk->targetsSize == 0) {
        //         UA_free(rk->targets.array);
        //         rk->targets.array = NULL;
        //     }
        //     return retval;
        // }
        // rk->targetsSize++;
        // return UA_STATUSCODE_GOOD;
    }

    node_ptr_u Base::createNodePointerUnion(const NodeID& nodeID)
    {
        node_ptr_u npu;
        if (nodeID.GetType() != node_id_type_e::NUMERIC)
        {
            npu.ID = &nodeID;
            npu.Immediate |= NODE_POINTER_TAG_NODE_ID;
            return npu;
        }

        if (nodeID.GetNamespaceIndex() < (0x01 << 6) && nodeID.GetID().Numeric < (0x01 << 24))
        {
            npu.Immediate  = ((uintptr_t)nodeID.GetID().Numeric) << 8;
            npu.Immediate |= ((uintptr_t)nodeID.GetNamespaceIndex()) << 2;
        }
        else
        {
            npu.ID = &nodeID;
            npu.Immediate |= NODE_POINTER_TAG_NODE_ID;
        }

        return npu;
    }

    Status Base::copyNodePointer(node_ptr_u& source, node_ptr_u* sink)
    {
        uint8_t tag = source.Immediate & NODE_POINTER_MASK;
        source.Immediate &= ~(uintptr_t)NODE_POINTER_MASK;

        // UA_NodePointer_copy
        switch (tag)
        {
        case NODE_POINTER_TAG_NODE:
            source.ID = &source.Node->mNodeID;
            goto NODE_ID;

        case NODE_POINTER_TAG_NODE_ID:
        NODE_ID:
            sink->ID = (NodeID*)malloc(sizeof(NodeID));
            if (sink->ID == nullptr)
            {
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            break;

        default:
        case NODE_POINTER_TAG_IMMEDIATE:
            *sink = source;
            break;
        }
    }
*/
}}