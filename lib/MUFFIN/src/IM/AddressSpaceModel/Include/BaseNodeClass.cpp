/**
 * @file BaseNodeClass.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 BaseNodeClass 클래스를 정의합니다.
 * 
 * @date 2024-10-25
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "BaseNodeClass.h"
#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"



namespace muffin { namespace im {

    BaseNodeClass::BaseNodeClass(std::shared_ptr<BaseNodeId> nodeId, const node_class_e nodeClass, const QualifiedName& browseName, const LocalizedText displayName)
        : mNodeId(nodeId)
        , mNodeClass(nodeClass)
        , mBrowseName(browseName)
        , mDisplayName(displayName)
    {
    }
    
    BaseNodeClass::~BaseNodeClass()
    {
    }

    std::shared_ptr<const BaseNodeId> BaseNodeClass::GetNodeID() const
    {
        return mNodeId;
    }

    const node_class_e& BaseNodeClass::GetNodeClass() const
    {
        return mNodeClass;
    }

    const QualifiedName& BaseNodeClass::GetBrowseName() const
    {
        return mBrowseName;
    }

    const LocalizedText& BaseNodeClass::GetDisplayName() const
    {
        return mDisplayName;
    }
}}