/**
 * @file ReferenceTypeNodeClass.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 기본 노드 클래스를 상속받는 참조 타입 노드 클래스를 선언합니다.
 * 
 * @date 2024-10-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "BaseNodeClass.h"



namespace muffin { namespace im {

    class ReferenceTypeNodeClass : public BaseNodeClass
    {
    public:
    /**
     * @todo 다음 매개변수는 자동으로 생성되어 <BaseNodeClass> 개체에 입력될 수 있도록 코드를 작성해야 합니다.
     *         - std::shared_ptr<BaseNodeId> nodeId
     *         - const node_class_e nodeClass
     *         - const QualifiedName& browseName
     *         - const LocalizedText displayName
     */
        ReferenceTypeNodeClass(const bool isAbstract, const bool isSymmetric);
        virtual ~ReferenceTypeNodeClass();
    };
}}