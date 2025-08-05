/**
 * @file NodeTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 NodeTable을 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "Common/Allocator/psramAllocator.h"
#include "IM/Node/Node.h"




namespace muffin { namespace ethernetIP {

    class NodeTable
    {
    public:
        NodeTable();
        virtual ~NodeTable();
    private:
        using NodeRef = im::Node;
    public:
        Status Update(NodeRef& node);
        Status Remove(NodeRef& node);
        void Clear();
    public:
        std::pair<Status, psramVector<im::Node*>> RetrieveEntireNode() const;
    
    private:
        /**
         * @brief 
         */
        psramVector<NodeRef*> mVectorNodeReference;

    };
}}


#endif