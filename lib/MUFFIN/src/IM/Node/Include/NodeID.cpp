/**
 * @file NodeID.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 식별자를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




/* For future MUFFIN code base
#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "NodeID.h"



namespace muffin { namespace im {

    NodeID::NodeID(const uint16_t namespaceIndex, const uint32_t id)
        : mNamespaceIndex(namespaceIndex)
        , mIdentifierType(node_id_type_e::NUMERIC)
    {
        mIdentifier.Numeric = id;

    #if defined(DEBUG)
        LOG_DEBUG(logger, "[NUMERIC NODE ID] Constructed at address: %p", this);
    #endif
    }

    NodeID::NodeID(const uint16_t namespaceIndex, const string_t& id)
        : mNamespaceIndex(namespaceIndex)
        , mIdentifierType(node_id_type_e::STRING)
    {
        mIdentifier.String = id;

    #if defined(DEBUG)
        LOG_DEBUG(logger, "[STRING NODE ID] Constructed at address: %p", this);
    #endif
    }

    NodeID::NodeID(const NodeID&& obj) noexcept
        : mNamespaceIndex(std::move(obj.mNamespaceIndex))
        , mIdentifierType(std::move(obj.mIdentifierType))
        , mIdentifier(std::move(obj.mIdentifier))
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed by Move from %p to %p", &obj, this);
    #endif
    }

    NodeID::NodeID(const NodeID& obj)
        : mNamespaceIndex(obj.mNamespaceIndex)
        , mIdentifierType(obj.mIdentifierType)
        , mIdentifier(obj.mIdentifier)
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed by Copy from %p to %p", &obj, this);
    #endif
    }

    NodeID::~NodeID()
    {
        if (mIdentifierType == node_id_type_e::STRING)
        {
            free(mIdentifier.String.Data);
        }

    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    uint16_t NodeID::GetNamespaceIndex() const
    {
        return mNamespaceIndex;
    }

    node_id_type_e NodeID::GetType() const
    {
        return mIdentifierType;
    }

    const node_id_u& NodeID::GetID() const
    {
        return mIdentifier;
    }
}}
*/