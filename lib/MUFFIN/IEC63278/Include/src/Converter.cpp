/**
 * @file Converter.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../Converter.hpp" 



namespace muffin { namespace aas {


    key_types_e ConvertToKeyType(const char* keyType)
    {
        if (strcmp(keyType, "Referable") == 0)
        {
            return key_types_e::REFERABLE;
        }
        else if (strcmp(keyType, "FragmentReference") == 0)
        {
            return key_types_e::FRAGMENT_REFERENCE;
        }
        else if (strcmp(keyType, "GlobalReference") == 0)
        {
            return key_types_e::GLOBAL_REFERENCE;
        }
        else if (strcmp(keyType, "AssetAdministrationShell") == 0)
        {
            return key_types_e::ASSET_ADMINISTRATION_SHELL;
        }
        else if (strcmp(keyType, "ConceptDescription") == 0)
        {
            return key_types_e::CONCEPT_DESCRIPTION;
        }
        else if (strcmp(keyType, "Identifiable") == 0)
        {
            return key_types_e::IDENTIFIABLE;
        }
        else if (strcmp(keyType, "Submodel") == 0)
        {
            return key_types_e::SUBMODEL;
        }
        else if (strcmp(keyType, "AnnotatedRelationshipElement") == 0)
        {
            return key_types_e::ANNOTATED_RELATIONSHIP_ELEMENT;
        }
        else if (strcmp(keyType, "BasicEventElement") == 0)
        {
            return key_types_e::BASIC_EVENT_ELEMENT;
        }
        else if (strcmp(keyType, "Blob") == 0)
        {
            return key_types_e::BLOB;
        }
        else if (strcmp(keyType, "Capability") == 0)
        {
            return key_types_e::CAPABILITY;
        }
        else if (strcmp(keyType, "DataElement") == 0)
        {
            return key_types_e::DATA_ELEMENT;
        }
        else if (strcmp(keyType, "Entity") == 0)
        {
            return key_types_e::ENTITY;
        }
        else if (strcmp(keyType, "EventElement") == 0)
        {
            return key_types_e::EVENT_ELEMENT;
        }
        else if (strcmp(keyType, "File") == 0)
        {
            return key_types_e::FILE;
        }
        else if (strcmp(keyType, "MultiLanguageProperty") == 0)
        {
            return key_types_e::MULTI_LANGUAGE_PROPERTY;
        }
        else if (strcmp(keyType, "Operation") == 0)
        {
            return key_types_e::OPERATION;
        }
        else if (strcmp(keyType, "Property") == 0)
        {
            return key_types_e::PROPERTY;
        }
        else if (strcmp(keyType, "Range") == 0)
        {
            return key_types_e::RANGE;
        }
        else if (strcmp(keyType, "ReferenceElement") == 0)
        {
            return key_types_e::REFERENCE_ELEMENT;
        }
        else if (strcmp(keyType, "RelationshipElement") == 0)
        {
            return key_types_e::RELATIONSHIP_ELEMENT;
        }
        else if (strcmp(keyType, "SubmodelElement") == 0)
        {
            return key_types_e::SUBMODEL_ELEMENT;
        }
        else
        {
            ASSERT(false, "UNDEFINED KEY TYPE: %s", keyType);
            log_e("Unknown key type: %s", keyType);
        }
    }
}}