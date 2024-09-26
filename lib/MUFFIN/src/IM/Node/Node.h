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
#include "Method.h"
#include "Variable.h"



namespace muffin { namespace im {

    class Node
    {
    public:
        Node(const std::string& nodeID, const std::string& uid, const std::string& pid, const data_type_e dataType);
        virtual ~Node();
    private:
        const std::string mNodeID;
        const std::string mUID;
        const std::string mPID;
    public:
        Variable mVariableNode;
        Method mMethodNode;
    };
}}