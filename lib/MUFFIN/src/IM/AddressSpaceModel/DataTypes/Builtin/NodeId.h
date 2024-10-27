/**
 * @file NodeId.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief OPC UA 표준의 NodeId DataType을 정의합니다.
 * @note MUFFIN 프레임워크 코딩 표준에서는 단어 끝에 붙는 약자는 모두 대문자여야 합니다.
 *       다만 OPC UA 표준에서는 "NodeId"로 정의되어 있기 때문에 MUFFIN 코딩 표준을 덮어씁니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */





#pragma once

#include <sys/_stdint.h>

#include "Common/Logger/Logger.h"



namespace muffin { namespace im {

    typedef enum class IdTypeEnum
        : uint8_t
    {
        NUMERIC   = 0,
        STRING    = 1,
        GUID      = 2,
        OPAQUE    = 3
    } id_type_e;

    typedef enum class NodeClassEnum
    {
        UNSPECIFIED     =   0,  // No value is specified.
        OBJECT          =   1,  // The Node is an Object.
        VARIABLE        =   2,  // The Node is a Variable.
        METHOD          =   4,  // The Node is a Method.
        OBJECT_TYPE     =   8,  // The Node is an ObjectType.
        VARIABLE_TYPE   =  16,  // The Node is a VariableType.
        REFERENCE_TYPE  =  32,  // The Node is a ReferenceType.
        DATA_TYPE       =  64,  // The Node is a DataType.
        VIEW            = 128   // The Node is a View.
    } node_class_e;


    class BaseNodeId
    {
    public:
        virtual ~BaseNodeId() = default;
    };
    

    template <typename T>
    class NodeId : public BaseNodeId
    {
    public:
        NodeId(const uint16_t namespaceIndex, const id_type_e idType, const T& identifier)
            : mNamespaceIndex(namespaceIndex)
            , mIdType(idType)
            , mIdentifier(identifier)
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Constructed at address: %p", this);
        #endif
        }

        NodeId(const NodeId& obj)
            : mNamespaceIndex(obj.mNamespaceIndex)
            , mIdType(obj.mIdType)
            , mIdentifier(obj.mIdentifier)
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Constructed at address: %p", this);
        #endif
        }

        virtual ~NodeId() override
        {
        #if defined(DEBUG)
            LOG_VERBOSE(logger, "Destroyed at address: %p", this);
        #endif
        }

        uint16_t GetNamespaceIndex() const
        {
            return mNamespaceIndex;
        }

        id_type_e GetIdType() const
        {
            return mIdType;
        }

        T GetIdentifier() const
        {
            return mIdentifier;
        }

    private:
        const uint16_t mNamespaceIndex;
        const id_type_e mIdType;
        const T mIdentifier;
    };
}}