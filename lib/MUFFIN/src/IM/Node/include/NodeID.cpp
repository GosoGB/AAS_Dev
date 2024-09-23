/**
 * @file NodeID.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 식별자를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>
#include "NodeID.h"



namespace muffin { namespace im {

    NodeID::NodeID(const uint16_t namespaceIndex, const uint32_t id)
        : mNamespaceIndex(namespaceIndex)
        , mIdentifierType(node_id_type_e::NUMERIC)
    {
        mIdentifier.Numeric = id;
    }

    NodeID::NodeID(const uint16_t namespaceIndex, std::string& id)
        : mNamespaceIndex(namespaceIndex)
        , mIdentifierType(node_id_type_e::STRING)
    {
        string_t str;
        str.Length = id.length();
        str.Data = reinterpret_cast<uint8_t*>(&id[0]);
        mIdentifier.String = str;
    }

    NodeID::NodeID(const NodeID&& obj) noexcept
        : mNamespaceIndex(std::move(obj.mNamespaceIndex))
        , mIdentifierType(std::move(obj.mIdentifierType))
        , mIdentifier(std::move(obj.mIdentifier))
    {}

    NodeID::NodeID(const NodeID& obj)
        : mNamespaceIndex(obj.mNamespaceIndex)
        , mIdentifierType(obj.mIdentifierType)
        , mIdentifier(obj.mIdentifier)
    {}

    NodeID::~NodeID()
    {
        ;
    }

    uint16_t NodeID::GetNamespaceIndex() const
    {
        return mNamespaceIndex;
    }

    node_id_type_e NodeID::GetIdentifierType() const
    {
        return mIdentifierType;
    }

    const node_id_u& NodeID::GetID() const
    {
        return mIdentifier;
    }
}}