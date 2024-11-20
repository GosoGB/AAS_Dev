/**
 * @file NodeStore.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 개체를 관리하는 백엔드 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <string>

#include "Common/Status.h"
#include "Jarvis/Config/Information/Node.h"
#include "Node.h"



namespace muffin { namespace im {

    class NodeStore
    {
    public:
        NodeStore(NodeStore const&) = delete;
        void operator=(NodeStore const&) = delete;
        static NodeStore* CreateInstanceOrNULL();
        static NodeStore& GetInstance();
    public:
        std::map<std::string, Node>::iterator begin();
        std::map<std::string, Node>::iterator end();
        std::map<std::string, Node>::const_iterator begin() const;
        std::map<std::string, Node>::const_iterator end() const;

    public:
        std::vector<Node*> GetCyclicalNode();
    private:
        NodeStore();
        virtual ~NodeStore();
    private:
        static NodeStore* mInstance;
    
    public:
        Status Create(const jarvis::config::Node* cin);
        Status Remove(const std::string& nodeID);
        void Clear();
        std::pair<Status, Node*> GetNodeReference(const std::string& nodeID);
        std::pair<Status, Node*> GetNodeReferenceUID(const std::string& UID);
    private:
        std::map<std::string, Node> mMapNode;
    };
}}