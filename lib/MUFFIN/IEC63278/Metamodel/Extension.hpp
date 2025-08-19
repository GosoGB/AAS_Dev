/**
 * @file Extension.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * Single extension of an element.
 * 
 * @note
 * The cardinality of attribute 'refersTo' is limited to 0..1 due to memory usage.
 * 
 * @date 2025-08-04
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>

#include "./Abstract/HasSemantics.hpp"
#include "./Reference.hpp"
#include "./TypeDefinitions.hpp"
#include "./Utility/XsdTypeMapper.hpp"

#include "Common/STL/bitset.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class ExtensionBase
    {
    public:
        virtual ~ExtensionBase() = default;
    public:
        virtual void SetReference(const Reference& refersTo) {}
        virtual void SetSemanticID(const Reference& semanticId) {}
    public:
        virtual const char* GetName() const noexcept { return nullptr; }
        virtual const data_type_def_xsd_e* GetValueType() const noexcept { return nullptr; }
        virtual const Reference* GetRefersToOrNULL() const { return nullptr; }
        virtual const Reference* GetSemanticID() const noexcept { return nullptr; }
        virtual psram::unique_ptr<ExtensionBase> Clone() const
        {
            return psram::unique_ptr<ExtensionBase>();
        }
    };


    template<data_type_def_xsd_e xsd>
    class Extension : public ExtensionBase, public HasSemantics
    {
    public:
        Extension(const psram::string& name)
            : mName(name)
        {}

        Extension(const psram::string& name, const data_type_def_xsd_e valueType)
            : mName(name)
            , mValueType(valueType)
        {
            mHasAttribute.set(static_cast<uint8_t>(flag_e::VALUE));
            mHasAttribute.set(static_cast<uint8_t>(flag_e::VALUE_TYPE));
        }

        Extension(const Extension& other)
            : HasSemantics(other)
            , mName(other.mName)
            , mValueType(other.mValueType)
            , mValue(other.mValue)
            , mHasAttribute(other.mHasAttribute)
        {
            mRefersTo = psram::make_unique<Reference>(*other.mRefersTo.get());
        }

        Extension& operator=(const Extension& other)
        {
            if (this != &other)
            {
                HasSemantics::operator=(other);
                mName = other.mName;
                mValueType = other.mValueType;
                mValue = other.mValue;
                mRefersTo = other.mRefersTo;
                mHasAttribute = other.mHasAttribute;
            }
            return *this;
        }

        psram::unique_ptr<ExtensionBase> Clone() const override
        {
            using namespace psram;
            return make_unique<ExtensionBase>(*make_unique<Extension<xsd>>(*this));
        }

    public:
        void SetValue(typename xsd_type_mapper<xsd>::type value)
        {
            mHasAttribute.set(static_cast<uint8_t>(flag_e::VALUE));
            mValue = value;
        }

        void SetReference(const Reference& refersTo) override
        {
            mHasAttribute.set(static_cast<uint8_t>(flag_e::REFERS_TO));
            mRefersTo = psram::make_unique<Reference>(refersTo);
        }

        void SetSemanticID(const Reference& semanticId) override
        {
            Reference semanticID = semanticId;
            mSemanticID.reset(&semanticID);
        }

    public:
        const char* GetName() const noexcept override
        {
            return mName.c_str();
        }

        const data_type_def_xsd_e* GetValueType() const noexcept override
        {
            return &mValueType;
        }

        const typename xsd_type_mapper<xsd>::type* GetValueOrNULL() const noexcept
        {
            const bool ret = mHasAttribute.test(static_cast<uint8_t>(flag_e::VALUE_TYPE));
            if (ret == true)
            {
                return &mValue;
            }
            else
            {
                return nullptr;
            }
        }

        const Reference* GetRefersToOrNULL() const override
        {
            return mRefersTo ? mRefersTo.get() : nullptr;
        }

        const Reference* GetSemanticID() const noexcept override
        {
            return mSemanticID.get();
        }

    private:
        psram::string mName;
        data_type_def_xsd_e mValueType;
        typename xsd_type_mapper<xsd>::type mValue;
        psram::unique_ptr<Reference> mRefersTo;
    private:
        typedef enum class HasAttributeFlagEnum
        {
            VALUE_TYPE = 0,
            VALUE,
            REFERS_TO,
            TOP
        } flag_e;
        bitset<static_cast<uint8_t>(flag_e::TOP)> mHasAttribute;
    };
}}