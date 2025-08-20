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
        }

        QualifiedName(const QualifiedName& obj)
            : mNamespaceIndex(obj.mNamespaceIndex)
            , mName(obj.mName)
        {
        }

        virtual ~QualifiedName()
        {
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