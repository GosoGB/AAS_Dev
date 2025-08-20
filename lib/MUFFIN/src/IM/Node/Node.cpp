/**
 * @file Node.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Node 클래스를 정의합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Node.h"



namespace muffin { namespace im {

    Node::Node(const jvs::config::Node* cin)
        : VariableNode(cin)
        , mCIN(cin)
    {
    }

    const char* Node::GetNodeID() const
    {
        return mCIN->GetNodeID().second;
    }

    mqtt::topic_e Node::GetTopic() const
    {
        return mCIN->GetTopic().second;
    }

    bool Node::IsArrayNode() const
    {
        return (mCIN->GetArrayIndex().second.size() != 0);
    }

    bool Node::HasAttributeEvent() const
    {
        return mCIN->GetAttributeEvent().second;
    }
}}