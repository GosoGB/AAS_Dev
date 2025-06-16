/**
 * @file Node.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Node 클래스를 선언합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "Protocol/MQTT/Include/TypeDefinitions.h"
#include "JARVIS/Config/Information/Node.h"
#include "Method.h"
#include "Variable.h"



namespace muffin { namespace im {

    class Node
    {
    public:
        explicit Node(const jvs::config::Node* cin);
        ~Node() {}
    public:
        const char* GetNodeID() const;
        mqtt::topic_e GetTopic() const;
        bool HasAttributeEvent() const;
    public:
        Variable VariableNode;
        Method MethodNode;
    private:
        const jvs::config::Node* const mCIN;
    };
}}