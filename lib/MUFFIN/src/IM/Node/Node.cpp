/**
 * @file Node.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Node 클래스를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Node.h"



namespace muffin { namespace im {

    Node::Node(const std::string& nodeID, const std::string& uid, const std::string& pid, const data_type_e dataType)
        : mNodeID(nodeID)
        , mUID(uid)
        , mPID(pid)
        , VariableNode(dataType)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Node::~Node()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    const std::string& Node::GetNodeID() const
    {
        return mNodeID;
    }
}}