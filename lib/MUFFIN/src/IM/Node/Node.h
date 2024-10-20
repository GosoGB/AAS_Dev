/**
 * @file Node.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Node 클래스를 선언합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Config/Information/Node.h"
#include "Method.h"
#include "Variable.h"



namespace muffin { namespace im {

    class Node
    {
    public:
        explicit Node(const jarvis::config::Node* cin);
        virtual ~Node();
    public:
        const std::string& GetNodeID() const;
        const std::string& GetUID() const;
    private:
        const std::string mNodeID;
        const std::string mDeprecableUID;
    public:
        Variable VariableNode;
        Method MethodNode;
    };
}}