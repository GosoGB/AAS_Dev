/**
 * @file TypeDefinitions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once



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
    typedef enum class AssetKind
    {
        Type,
        Instance
    } asset_kind_e;


    /**
     * @brief
     * Reference to either a model element of the same or another AAS or to an external entity.
     */
    typedef enum class ReferenceTypes
    {
        GlobalReference,
        ModelReference
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
    typedef enum class KeyTypes
    {
        Referable,
        FragmentReference,
        GlobalReference,
        AssetAdministrationShell,
        ConceptDescription,
        Identifiable,
        Submodel,
        AnnotatedRelationshipElement,
        BasicEventElement,
        Blob,
        Capability,
        DataElement,
        Entity,
        EventElement,
        File,
        MultiLanguageProperty,
        Operation,
        Property,
        Range,
        ReferenceElement,
        RelationshipElement,
        SubmodelElement
    } key_types_e;
}}