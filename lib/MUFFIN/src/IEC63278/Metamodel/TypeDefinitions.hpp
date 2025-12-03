/**
 * @file TypeDefinitions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * AAS 시스템 내에서 사용되는 concrete type 자료구조를 정의합니다.
 * 여기에는 구조체, 열거형 또는 상수 등이 포함됩니다.
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <cstdint>



namespace muffin { namespace aas {


    /**
     * @brief Enumeration for denoting whether an asset is a type asset or an instance asset.
     * @details
     *  @enum Type
     *      - hardware or software element which specifies the common attributes shared
     *        by all instances of the type
     *        [source: IEC TR 62390:2005-01, 3.1.25]
     *  @enum Instance
     *      - concrete, clearly identifiable component of a certain type
     *        [source: IEC 62890:2016, 3.1.16] 65/617/CDV
     */
    typedef enum class AssetKindEnum
        : uint8_t
    {
        TYPE,
        INSTANCE,
        ROLE,
        NOT_APPLICABLE,
        TOP
    } asset_kind_e;


    /**
     * @brief
     * Reference to either a model element of the same or another AAS or to an external entity.
     */
    typedef enum class ReferenceTypesEnum
        : uint8_t
    {
        EXTERNAL_REFERENCE,
        MODEL_REFERENCE,
        TOP
    } reference_types_e;


    /**
     * @brief Enumeration of different key value types within @class Key.
     * 
     * @note
     * The former 'KeyElements' is renamed to 'KeyTypes' in Metamodel V3.0RC02 w.r.t. V2.0.1.
     * 
     * @note
     * The elements remain except for new 'SubmodelElementList', and renamed submodel elements 
     * 'Event' and 'BasicEvent' to 'EventElement' and 'BasicEventElement'.
     */
    typedef enum class KeyTypesEnum
        : uint8_t
    {
        ANNOTATED_RELATIONSHIP_ELEMENT,
        ASSET_ADMINISTRATION_SHELL,
        BASIC_EVENT_ELEMENT,
        BLOB,
        CAPABILITY,
        CONCEPT_DESCRIPTION,
        DATA_ELEMENT,
        ENTITY,
        EVENT_ELEMENT,
        FILE,
        FRAGMENT_REFERENCE,
        GLOBAL_REFERENCE,
        IDENTIFIABLE,
        MULTI_LANGUAGE_PROPERTY,
        OPERATION,
        PROPERTY,
        RANGE,
        REFERABLE,
        REFERENCE_ELEMENT,
        RELATIONSHIP_ELEMENT,
        SUBMODEL,
        SUBMODEL_ELEMENT,
        SUBMODEL_ELEMENT_COLLECTION,
        SUBMODEL_ELEMENT_LIST,
        TOP // sentinel value indicating the value's  undefined
    } key_types_e;


    typedef enum class DataTypeDefXsdEnum
        : uint8_t
    {
        STRING,                 // Character string (but not all Unicode character strings)
        BOOLEAN,                // true, false
        DECIMAL,                // Arbitrary-precision decimal numbers
        INTEGER,                // Arbitrary-size integer numbers
        DOUBLE,                 // 64-bit floating point numbers incl. ±Inf, ±0, NaN
        FLOAT,                  // 32-bit floating point numbers incl. ±Inf, ±0, NaN
        DATE,                   // Date (yyyy‑mm‑dd), optional timezone
        TIME,                   // Time (hh:mm:ss.sss…), optional timezone
        DATETIME,               // Date and time, optional timezone
        G_YEAR,                 // Gregorian calendar year
        G_MONTH,                // Gregorian month
        G_DAY,                  // Gregorian day
        G_YEAR_MONTH,           // Gregorian year and month
        G_MONTH_DAY,            // Gregorian month and day
        DURATION,               // General duration
        BYTE,                   // 8‑bit signed integer (‑128 to 127)
        SHORT,                  // 16‑bit signed integer (‑32,768 to 32,767)
        INT,                    // 32‑bit signed integer
        LONG,                   // 64‑bit signed integer
        UNSIGNED_BYTE,          // 8‑bit unsigned integer (0 to 255)
        UNSIGNED_SHORT,         // 16‑bit unsigned integer (0 to 65,535)
        UNSIGNED_INT,           // 32‑bit unsigned integer (0 to 4,294,967,295)
        UNSIGNED_LONG,          // 64‑bit unsigned integer
        POSITIVE_INTEGER,       // Integer > 0
        NON_NEGATIVE_INTEGER,   // Integer ≥ 0
        NEGATIVE_INTEGER,       // Integer < 0
        NON_POSITIVE_INTEGER,   // Integer ≤ 0
        HEX_BINARY,             // Hex-encoded binary data
        BASE64_BINARY,          // Base64-encoded binary
        ANY_URI                 // Absolute or relative URI/IRI
    } data_type_def_xsd_e;


    /**
     * @brief
     * Data types for RDF.
     * 
     * @note
     * Currently only langString is supported.
     */
    typedef enum class DataTypeDefRdf
        : uint8_t
    {
        LANG_STRING
    } data_type_def_rdf_e;


    typedef enum class ModelingKindEnum
        : uint8_t
    {
        TEMPLATE,
        INSTANCE,
        TOP
    } modeling_kind_e;


    typedef enum class QualifierKindEnum
        : uint8_t
    {
        VALUE_QUALIFIER,
        CONCEPT_QUALIFIER,
        TEMPLATE_QUALIFIER,
        TOP
    } qualifier_kind_e;


    typedef enum class DirectionEnum
        : uint8_t
    {
        INPUT,
        OUTPUT,
        TOP
    } direction_e;


    typedef enum class StateOfEventEnum
        : uint8_t
    {
        ON,
        OFF,
        TOP
    } state_of_event_e;
    

    typedef enum class EntityTypeEnum
        : uint8_t
    {
        CO_MANAGED_ENTITY,
        SELF_MANAGED_ENTITY,
        TOP
    } entity_type_e;


    typedef enum class AasSubmodelElementsEnum
        : uint8_t
    {
        ANNOTATED_RELATIONSHIP_ELEMENT,
        BASIC_EVENT_ELEMENT,
        BLOB,
        CAPABILITY,
        DATA_ELEMENT,
        ENTITY,
        EVENT_ELEMENT,
        FILE,
        MULTI_LANGUAGE_PROPERTY,
        OPERATION,
        PROPERTY,
        RANGE,
        REFERENCE_ELEMENT,
        RELATIONSHIP_ELEMENT,
        SUBMODEL_ELEMENT,
        SUBMODEL_ELEMENT_COLLECTION,
        SUBMODEL_ELEMENT_LIST,
        TOP
    } aas_submodel_elements_e;


    typedef enum class FragmentKeysEnum
        : uint8_t
    {
        ANNOTATED_RELATIONSHIP_ELEMENT,
        BASIC_EVENT_ELEMENT,
        BLOB,
        CAPABILITY,
        DATA_ELEMENT,
        ENTITY,
        EVENT_ELEMENT,
        FILE,
        FRAGMENT_REFERENCE,
        MULTI_LANGUAGE_PROPERTY,
        OPERATION,
        PROPERTY,
        RANGE,
        REFERENCE_ELEMENT,
        RELATIONSHIP_ELEMENT,
        SUBMODEL_ELEMENT,
        SUBMODEL_ELEMENT_COLLECTION,
        SUBMODEL_ELEMENT_LIST,
        TOP
    } fragment_keys_e;


    typedef enum class DataTypeDefRdfEnum
        : uint8_t
    {
        LANG_STRING,
        TOP
    } data_type_def_rdf_e;
}}