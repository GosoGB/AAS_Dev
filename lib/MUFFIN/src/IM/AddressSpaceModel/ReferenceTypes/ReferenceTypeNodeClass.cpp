/**
 * @file ReferenceTypeNodeClass.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 기본 노드 클래스를 상속받는 참조 타입 노드 클래스를 정의합니다.
 * 
 * @date 2024-10-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <memory>

#include "ReferenceTypeNodeClass.h"



namespace muffin { namespace im {

    ReferenceTypeNodeClass::ReferenceTypeNodeClass(std::shared_ptr<BaseNodeId> nodeId, 
                                                   const node_class_e nodeClass, 
                                                   const QualifiedName& browseName, 
                                                   const LocalizedText displayName, 
                                                   const bool isAbstract, 
                                                   const bool isSymmetric)
        : BaseNodeClass(nodeId, nodeClass, browseName, displayName)
        , mIsAbstract(isAbstract)
        , mIsSymmetric(isSymmetric)
    {
        ;
    }


    ReferenceTypeNodeClass referenceTypeNodeClass(
        std::make_shared<NodeId<uint8_t>>(0, id_type_e::NUMERIC, 31),
        node_class_e::REFERENCE_TYPE,
        QualifiedName(0, "References"),
        LocalizedText(locale_id_e::EN_US, "References"),
        true,
        true
    );
}}