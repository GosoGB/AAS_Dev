/**
 * @file XsdTypeMapper.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Provides compile-time mapping and validation between C++ types and XSD data types
 * as defined in data_type_def_xsd_e.
 * 
 * @date 2025-07-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>
#include <cstdint>
#include <type_traits>

#include "../TypeDefinitions.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {

    /**
     * @brief Type trait to check if a C++ type is a valid XSD-mappable type.
     */
    template<typename T>
    struct is_valid_xsd_type : std::false_type {};
    template<> struct is_valid_xsd_type<psram::string>  : std::true_type {};
    template<> struct is_valid_xsd_type<bool>           : std::true_type {};
    template<> struct is_valid_xsd_type<double>         : std::true_type {};
    template<> struct is_valid_xsd_type<float>          : std::true_type {};
    template<> struct is_valid_xsd_type<int8_t>         : std::true_type {};
    template<> struct is_valid_xsd_type<int16_t>        : std::true_type {};
    template<> struct is_valid_xsd_type<int32_t>        : std::true_type {};
    template<> struct is_valid_xsd_type<int64_t>        : std::true_type {};
    template<> struct is_valid_xsd_type<uint8_t>        : std::true_type {};
    template<> struct is_valid_xsd_type<uint16_t>       : std::true_type {};
    template<> struct is_valid_xsd_type<uint32_t>       : std::true_type {};
    template<> struct is_valid_xsd_type<uint64_t>       : std::true_type {};

    /**
     * @brief Maps a C++ type to its corresponding data_type_def_xsd_e enum value.
     */
    template<typename T> data_type_def_xsd_e get_xsd_type_from_cpp();
    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<psram::string>()
    {
        return data_type_def_xsd_e::STRING;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<bool>()
    {
        return data_type_def_xsd_e::BOOLEAN;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<double>()
    {
        return data_type_def_xsd_e::DOUBLE;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<float>()
    {
        return data_type_def_xsd_e::FLOAT;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<int8_t>()
    {
        return data_type_def_xsd_e::BYTE;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<int16_t>()
    {
        return data_type_def_xsd_e::SHORT;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<int32_t>()
    {
        return data_type_def_xsd_e::INT;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<int64_t>()
    {
        return data_type_def_xsd_e::LONG;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<uint8_t>()
    {
        return data_type_def_xsd_e::UNSIGNED_BYTE;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<uint16_t>()
    {
        return data_type_def_xsd_e::UNSIGNED_SHORT;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<uint32_t>()
    {
        return data_type_def_xsd_e::UNSIGNED_INT;
    }

    template<> inline data_type_def_xsd_e get_xsd_type_from_cpp<uint64_t>()
    {
        return data_type_def_xsd_e::UNSIGNED_LONG;
    }
}}