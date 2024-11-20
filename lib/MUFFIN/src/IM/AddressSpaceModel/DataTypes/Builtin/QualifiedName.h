/**
 * @file QualifiedName.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief OPC UA 표준의 QualifiedName DataType을 정의합니다.
 * 
 * @date 2024-10-25
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Logger/Logger.h"



namespace muffin { namespace im {

    class QualifiedName
    {
    public:
        QualifiedName(const uint16_t namespaceIndex, const std::string& name)
            : mNamespaceIndex(namespaceIndex)
            , mName(name)
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Constructed at address: %p", this);
        #endif
        }

        QualifiedName(const QualifiedName& obj)
            : mNamespaceIndex(obj.mNamespaceIndex)
            , mName(obj.mName)
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Constructed by Copy from %p to %p", &obj, this);
        #endif
        }

        virtual ~QualifiedName()
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Destroyed at address: %p", this);
        #endif
        }

        uint16_t GetNamespaceIndex() const
        {
            return mNamespaceIndex;
        }

        std::string GetName() const
        {
            return mName;
        }

    private:
        const uint16_t mNamespaceIndex;
        const std::string mName;
    };
}}