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
 * @date 2025-07-28
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


    template<data_type_def_xsd_e xsd>
    class Extension : public HasSemantics
    {
    public:
        Extension(const psram::string& name)
            : mName(name)
        {}

        Extension(const psram::string& name, const xsd valueType)
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
            , mRefersTo(other.mRefersTo)
            , mHasAttribute(other.mHasAttribute)
        {}

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
            return psram::make_unique<Extension<T>>(*this);
        }

    public:
        const char* GetName() const noexcept override
        {
            return mName.c_str();
        }

        bool GetValueType(data_type_def_xsd_e* valueType) const noexcept override
        {
            ASSERT((valueType != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

            const bool ret = mHasAttribute.test(static_cast<uint8_t>(flag_e::VALUE_TYPE));
            if (ret == true)
            {
                *valueType = mValueType;
            }
            return ret;
        }

        bool GetValue(typename xsd_type_mapper<xsd>::type* value) const noexcept
        {
            ASSERT((value != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

            const bool ret = mHasAttribute.test(static_cast<uint8_t>(flag_e::VALUE));
            if (ret == true)
            {
                *value = mValue;
            }
            return ret;
        }

        bool GetRefersTo(Reference* refersTo) const noexcept override
        {
            ASSERT((refersTo != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

            const bool ret = mHasAttribute.test(static_cast<uint8_t>(flag_e::REFERS_TO));
            if (ret == true)
            {
                *refersTo = mRefersTo; // Now copies a Reference object
            }
            return ret;
        }

    private:
        psram::string mName;
        data_type_def_xsd_e mValueType;
        typename xsd_type_mapper<xsd>::type mValue;
        Reference mRefersTo;
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