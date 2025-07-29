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


    class ExtensionBase
    {
    public:
        virtual ~ExtensionBase() = default;
        virtual const char* GetName() const noexcept = 0;
        virtual bool GetValueType(data_type_def_xsd_e* valueType) const noexcept = 0;
        virtual bool GetRefersTo(Reference* refersTo) const noexcept = 0;
        virtual psram::unique_ptr<ExtensionBase> Clone() const = 0;
    };


    template<typename T>
    class Extension : public ExtensionBase, public HasSemantics
    {
    private:
        static_assert(is_valid_xsd_type<T>::value, "THE TYPE IS NOT A VALID XSD-MAPPABLE TYPE");

    public:
        Extension(const psram::string& name)
            : mName(name)
        {}

        Extension(const psram::string& name, const T& value)
            : mName(name)
            , mValue(value)
        {
            mValueType = get_xsd_type_from_cpp<T>();
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

        bool GetValue(T* value) const noexcept
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
        T mValue;
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