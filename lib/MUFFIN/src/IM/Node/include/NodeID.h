/**
 * @file NodeID.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 식별자를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Assert.h"
#include "TypeDefinitions.h"



namespace muffin { namespace im {

    class NodeID
    {
    public:
        NodeID(const uint16_t namespaceIndex, const uint32_t id);
        NodeID(const uint16_t namespaceIndex, std::string& id);
        NodeID(const NodeID&& obj) noexcept;
        NodeID(const NodeID& obj);
        ~NodeID();
    public:
        uint16_t GetNamespaceIndex() const;
        node_id_type_e GetIdentifierType() const;
        const node_id_u& GetID() const;
    private:
        const uint16_t mNamespaceIndex;
        const node_id_type_e mIdentifierType;
        node_id_u mIdentifier;
    };
}}