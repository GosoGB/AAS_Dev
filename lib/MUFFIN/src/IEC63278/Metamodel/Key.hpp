/**
 * @file Key.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief A key is a reference to an element by its ID.
 * 
 * @note The cardinality of both Key::type and Key::value should be 1.
 * 
 * @date 2025-08-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>

#include "./TypeDefinitions.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Key
    {
    public:
        Key(key_types_e type, const psram::string& value)
            : mType(type)
            , mValue(value)
        {
            ASSERT((value.empty() == false), "IDENTIFIER CANNOT BE EMPTY");
        }

        Key(key_types_e type, psram::string&& value)
            : mType(type)
            , mValue(std::move(value))
        {
            ASSERT((mValue.empty() == false), "IDENTIFIER CANNOT BE EMPTY");
        }

        Key(Key const& other) = default;
        Key(Key&& other) noexcept = default;
        Key& operator=(Key const& other) = default;
        Key& operator=(Key&& other) noexcept = default;
        ~Key() noexcept = default;

    public:
        bool operator==(const Key& other) const
        {
            return mType == other.mType && mValue == other.mValue;
        }

        bool operator!=(const Key& other) const
        {
            return !(*this == other);
        }

    public:
        key_types_e GetType() const noexcept
        {
            return mType;
        }

        const psram::string& GetValue() const noexcept
        {
            return mValue;
        }

    private:
        key_types_e mType;
        psram::string mValue;
    };
}}