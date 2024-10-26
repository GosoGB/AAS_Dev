/**
 * @file BaseNodeClass.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 BaseNodeClass 클래스를 선언합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <memory>

#include "IM/DataTypes/BuiltinDataType/LocalizedText.h"
#include "IM/DataTypes/BuiltinDataType/NodeId.h"
#include "IM/DataTypes/BuiltinDataType/QualifiedName.h"



namespace muffin { namespace im {

    class BaseNodeClass
    {
    protected:
        BaseNodeClass(std::shared_ptr<BaseNodeId> nodeId, const node_class_e nodeClass, const QualifiedName& browseName, const LocalizedText displayName);
        virtual ~BaseNodeClass();
    public:
        std::shared_ptr<const BaseNodeId> GetNodeID() const;
        const node_class_e& GetNodeClass() const;
        const QualifiedName& GetBrowseName() const;
        const LocalizedText& GetDisplayName() const;
    protected:
        /**
         * @brief Mandatory Attributes
         */
        std::shared_ptr<BaseNodeId> mNodeId;
        const node_class_e mNodeClass;
        const QualifiedName mBrowseName;
        const LocalizedText mDisplayName;
        LocalizedText mDescription;
    protected:
        /**
         * @brief Optional Attributes
         */
        // AttributeWriteMask mWriteMask;
        // AttributeWriteMask mUserWriteMask;
        // RolePermissionType[] mRolePermissions;
        // RolePermissionType[] mUserRolePermissions;
        // AccessRestrictionType mAccessRestrictions;
    };
}}