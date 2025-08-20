/**
 * @file ReferenceTypeNodeClass.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 기본 노드 클래스를 상속받는 참조 타입 노드 클래스를 선언합니다.
 * 
 * @date 2024-10-26
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "IM/AddressSpaceModel/Include/BaseNodeClass.h"



namespace muffin { namespace im {

    class ReferenceTypeNodeClass : public BaseNodeClass
    {
    public:
        /**
         * @todo Attributes 노드로 정보를 받도록 해야 합니다...?
         */
        ReferenceTypeNodeClass(std::shared_ptr<BaseNodeId> nodeId, 
                               const node_class_e nodeClass, 
                               const QualifiedName& browseName, 
                               const LocalizedText displayName, 
                               const bool isAbstract, 
                               const bool isSymmetric);
        virtual ~ReferenceTypeNodeClass() {}
    protected:
        /**
         * @brief Mandatory Attributes
         */
        bool mIsAbstract;
        bool mIsSymmetric;
    protected:
        /**
         * @brief Optional Attributes
         */
        LocalizedText mInverseName;
    protected:
        /**
         * @brief References
         */
        // HasProperty,  0..*,  Used to identify the Properties (see 5.3.3.2). 
        // HasSubtype,   0..*,  Used to identify subtypes (see 5.3.3.3). 
    protected:
        /**
         * @brief Standard Properties
         */
        // std::string mNodeVersion;
    };


    extern ReferenceTypeNodeClass referenceTypeNodeClass;
}}