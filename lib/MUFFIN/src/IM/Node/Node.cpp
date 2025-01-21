/**
 * @file Node.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Node 클래스를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Node.h"



namespace muffin { namespace im {

    Node::Node(const jvs::config::Node* cin)
        : mNodeID(cin->GetNodeID().second)
        , mDeprecableUID(cin->GetDeprecableUID().second)
        , VariableNode(mNodeID, mDeprecableUID)
    {
        VariableNode.Init(cin);
    }

    Node::~Node()
    {
    }

    const std::string& Node::GetNodeID() const
    {
        return mNodeID;
    }

    const std::string& Node::GetUID() const
    {
        return mDeprecableUID;
    }
}}