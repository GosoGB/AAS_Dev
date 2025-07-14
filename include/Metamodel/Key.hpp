/**
 * @file Key.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief A key is a reference to an element by its ID.
 * 
 * @note The cardinality of both Key::type and Key::value should be 1.
 * 
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>

#include "./TypeDefinitions.hpp"



namespace muffin { namespace aas {


    class Key
    {
    public:
        Key() {}
        Key(key_types_e type, const std::string& value)
            : type(type)
            , value(value)
            {}
    public:
        void SetType(const key_types_e& type) { this->type = type; }
        void SetValue(const std::string& value) { this->value = value; }
    public:
        key_types_e GetType() const { return type; }
        std::string GetValue() const { return value; }
    private:
        key_types_e type;
        std::string value;
    };
}}