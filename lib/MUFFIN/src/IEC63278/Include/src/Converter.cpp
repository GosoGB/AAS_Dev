/**
 * @file Converter.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include <pgmspace.h>

#include "../Converter.hpp" 



namespace muffin { namespace aas {
    

    const char* KEY_TYPE_STRING[24] PROGMEM = {
        "Referable",
        "FragmentReference",
        "GlobalReference",
        "AssetAdministrationShell",
        "ConceptDescription",
        "Identifiable",
        "Submodel",
        "AnnotatedRelationshipElement",
        "BasicEventElement",
        "Blob",
        "Capability",
        "DataElement",
        "Entity",
        "EventElement",
        "File",
        "MultiLanguageProperty",
        "Operation",
        "Property",
        "Range",
        "ReferenceElement",
        "RelationshipElement",
        "SubmodelElement",
        "SubmodelElementCollection",
        "SubmodelElementList"
    };

    const char* REFERENCE_TYPES_STRING[2] PROGMEM = {
        "ExternalReference",
        "ModelReference"
    };

    const char* ASSET_KIND_STRING[2] PROGMEM = {
        "Type",
        "Instance"
    };

    const char* MODELING_KIND_STRING[2] PROGMEM = {
        "Template",
        "Instance"
    };

    const char* DATA_TYPE_DEF_XSD_STRING[34] PROGMEM = {
        "xs:string",
        "xs:boolean",
        "xs:decimal",
        "xs:integer",
        "xs:double",
        "xs:float",
        "xs:date",
        "xs:time",
        "xs:dateTime",
        "xs:dateTimeStamp",
        "xs:gYear",
        "xs:gMonth",
        "xs:gDay",
        "xs:gYearMonth",
        "xs:gMonthDay",
        "xs:duration",
        "xs:yearMonthDuration",
        "xs:dayTimeDuration",
        "xs:byte",
        "xs:short",
        "xs:int",
        "xs:long",
        "xs:unsignedByte",
        "xs:unsignedShort",
        "xs:unsignedInt",
        "xs:unsignedLong",
        "xs:positiveInteger",
        "xs:nonNegativeInteger",
        "xs:negativeInteger",
        "xs:nonPositiveInteger",
        // "xs:hexBinary",
        // "xs:base64Binary",
        "xs:anyURI"
        // "rdf:langString"
    };

    const char* QUALIFIER_KIND_STRING[3] PROGMEM = {
        "ValueQualifier",
        "ConceptQualifier",
        "TemplateQualifier"
    };


    key_types_e ConvertToKeyType(const char* keyType)
    {
        for (uint8_t idx = 0; idx < static_cast<uint8_t>(key_types_e::TOP); ++idx)
        {
            if (strcmp(KEY_TYPE_STRING[idx], keyType) == 0)
            {
                return static_cast<key_types_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED KEY TYPE: %s", keyType);
        log_e("UNDEFINED KEY TYPE: %s", keyType);
        return key_types_e::TOP;
    }


    reference_types_e ConvertToReferenceType(const char* referenceType)
    {
        for (uint8_t idx = 0; idx < static_cast<uint8_t>(reference_types_e::TOP); ++idx)
        {
            if (strcmp(REFERENCE_TYPES_STRING[idx], referenceType) == 0)
            {
                return static_cast<reference_types_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED REFERENCE TYPE: %s", referenceType);
        log_e("UNDEFINED REFERENCE TYPE: %s", referenceType);
        return reference_types_e::TOP;
    }


    asset_kind_e ConvertToAssetKindType(const char* assetKind)
    {
        for (uint8_t idx = 0; idx < static_cast<uint8_t>(asset_kind_e::TOP); ++idx)
        {
            if (strcmp(ASSET_KIND_STRING[idx], assetKind) == 0)
            {
                return static_cast<asset_kind_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED ASSET KIND TYPE: %s", assetKind);
        log_e("UNDEFINED ASSET KIND TYPE: %s", assetKind);
        return asset_kind_e::TOP;
    }


    modeling_kind_e ConvertToModelingKindType(const char* modelingKind)
    {
        for (uint8_t idx = 0; idx < static_cast<uint8_t>(modeling_kind_e::TOP); ++idx)
        {
            if (strcmp(MODELING_KIND_STRING[idx], modelingKind) == 0)
            {
                return static_cast<modeling_kind_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED MODELING KIND TYPE: %s", modelingKind);
        log_e("UNDEFINED MODELING KIND TYPE: %s", modelingKind);
        return modeling_kind_e::TOP;
    }


    data_type_def_xsd_e ConvertToDataTypeDefXSD(const char* xsd)
    {
        const uint8_t MAX_ITERATION = static_cast<uint8_t>(data_type_def_xsd_e::ANY_URI) + 1;
        for (uint8_t idx = 0; idx < MAX_ITERATION; ++idx)
        {
            if (strcmp(DATA_TYPE_DEF_XSD_STRING[idx], xsd) == 0)
            {
                return static_cast<data_type_def_xsd_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED DATA TYPE DEFINTION: %s", xsd);
        log_e("UNDEFINED DATA TYPE DEFINTION: %s", xsd);
        /**
         * @todo need to refactor the function signature
         */
        return data_type_def_xsd_e::STRING;
    }

    qualifier_kind_e ConvertToQualifierKindType(const char* qualifierKind)
    {
        for (uint8_t idx = 0; idx < static_cast<uint8_t>(qualifier_kind_e::TOP); ++idx)
        {
            if (strcmp(QUALIFIER_KIND_STRING[idx], qualifierKind) == 0)
            {
                return static_cast<qualifier_kind_e>(idx);
            }
        }

        ASSERT(false, "UNDEFINED QUALIFIER KIND TYPE: %s", qualifierKind);
        log_e("UNDEFINED QUALIFIER KIND TYPE: %s", qualifierKind);
        return qualifier_kind_e::TOP;
    }


    psram::string ConvertToString(const key_types_e keyType)
    {
        ASSERT((static_cast<uint8_t>(keyType) < static_cast<uint8_t>(KeyTypes::TOP)),
                "UNDEFINED KEY TYPE: %u", static_cast<uint8_t>(keyType));
        return KEY_TYPE_STRING[static_cast<uint8_t>(keyType)];
    }


    psram::string ConvertToString(const reference_types_e refType)
    {
        ASSERT((static_cast<uint8_t>(refType) < 2), "UNDEFINED VALUE FOR REFERENCE TYPE");
        return REFERENCE_TYPES_STRING[static_cast<uint8_t>(refType)];
    }


    psram::string ConvertToString(const asset_kind_e assetKind)
    {
        ASSERT((static_cast<uint8_t>(assetKind) < 2), "UNDEFINED VALUE FOR ASSET KIND");
        return ASSET_KIND_STRING[static_cast<uint8_t>(assetKind)];
    }


    psram::string ConvertToString(const modeling_kind_e modelingKind)
    {
        ASSERT((static_cast<uint8_t>(modelingKind) < 2), "UNDEFINED VALUE FOR MODELING TYPE");
        return MODELING_KIND_STRING[static_cast<uint8_t>(modelingKind)];
    }


    psram::string ConvertToString(const data_type_def_xsd_e xsd)
    {
        ASSERT(
            (static_cast<uint8_t>(xsd) < static_cast<uint8_t>(data_type_def_xsd_e::ANY_URI) + 1),
            "UNDEFINED VALUE FOR XSD DATA TYPE: %d", static_cast<uint8_t>(xsd)
        );

        return DATA_TYPE_DEF_XSD_STRING[static_cast<uint8_t>(xsd)];
    }


    psram::string ConvertToString(const qualifier_kind_e qualifierKind)
    {
        ASSERT((
            static_cast<uint8_t>(qualifierKind) < 
            static_cast<uint8_t>(data_type_def_xsd_e::NON_POSITIVE_INTEGER) + 1),
            "UNDEFINED VALUE FOR QUALIFIER KIND TYPE"
        );
        return QUALIFIER_KIND_STRING[static_cast<uint8_t>(qualifierKind)];
    }
}}