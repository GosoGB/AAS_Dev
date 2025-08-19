/**
 * @file XsdTypeMapper.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Provides compile-time mapping and validation between C++ types and XSD data types
 * as defined in data_type_def_xsd_e.
 * 
 * @note
 * Following DataTypeDefXsd will be approximately represented
 *  - xs:decimal  <-- double
 *  - xs:integer  <-- int64_t
 * 
 * Following DataTypeDefXsd will require parsing for calculations
 *  - xs:date
 *  - xs:time
 *  - xs:datetime
 *  - xs:datetimestamp
 *  - xs:
 *  - xs:
 * 
 * @date 2025-08-04
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


    template <data_type_def_xsd_e T>
    struct xsd_type_mapper;

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::STRING>
    {
        using type = psram::string;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::BOOLEAN>
    {
        using type = bool;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DECIMAL>
    {
        using type = double; // represented approximately
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::INTEGER>
    {
        using type = int64_t; // represented approximately
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DOUBLE>
    {
        using type = double;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::FLOAT>
    {
        using type = float;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DATE>
    {
        using type = psram::string; // parsing required for calculation
    };
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::TIME>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DATETIME>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DATETIMESTAMP>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::G_YEAR>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::G_MONTH>
    {
        using type = psram::string; // parsing required for calculation
    };
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::G_DAY>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::G_YEAR_MONTH>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::G_MONTH_DAY>
    {
        using type = psram::string; // parsing required for calculation
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DURATION>
    {
        using type = psram::string; // parsing required for calculation
    };
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::YEAR_MONTH_DURATION>
    {
        using type = psram::string; // parsing required for calculation
    };
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::DAYTIME_DURATION>
    {
        using type = psram::string; // parsing required for calculation
    };
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::BYTE>
    {
        using type = int8_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::SHORT>
    {
        using type = int16_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::INT>
    {
        using type = int32_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::LONG>
    {
        using type = int64_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::UNSIGNED_BYTE>
    {
        using type = uint8_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::UNSIGNED_SHORT>
    {
        using type = uint16_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::UNSIGNED_INT>
    {
        using type = uint32_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::UNSIGNED_LONG>
    {
        using type = uint64_t;
    };
    

    /**
     * @todo POSITIVE_INTEGER > 0 으로 범위 제한을 추가해야 합니다.
     */
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::POSITIVE_INTEGER>
    {
        using type = uint64_t;
    };

    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::NON_NEGATIVE_INTEGER>
    {
        using type = uint64_t;
    };

    /**
     * @todo NEGATIVE_INTEGER < 0 으로 범위 제한을 추가해야 합니다.
     */
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::NEGATIVE_INTEGER>
    {
        using type = int64_t;
    };

    /**
     * @todo NON_POSITIVE_INTEGER ≤ 0 으로 범위 제한을 추가해야 합니다.
     */
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::NON_POSITIVE_INTEGER>
    {
        using type = int64_t;
    };

    // HEX_BINARY,             // Hex-encoded binary data
    // BASE64_BINARY,          // Base64-encoded binary
    
    template <>
    struct xsd_type_mapper<data_type_def_xsd_e::ANY_URI>
    {
        using type = psram::string; // Absolute or relative URI/IRI
    };

    // LANG_STRING             // String with language tag (RDF/Turtle syntax)
}}