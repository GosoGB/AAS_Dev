/**
 * @file SubmodelsDeserializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Include/Converter.hpp"
#include "../Include/SubmodelsDeserializer.hpp"



namespace muffin { namespace aas {

    
    psram::unique_ptr<Submodel> SubmodelsDeserializer::Parse(const JsonObject payload)
    {
        ASSERT(("Submodel" == payload["modelType"]), "INVALID VALUE FOR ATTRIBUTE 'modelType'");

        ASSERT((payload.containsKey("id")), "MISSING MANDATORY ATTRIBUTE 'id'");
        const char* id = payload["id"].as<const char*>();
        ASSERT((id != nullptr), "ATTRIBUTE 'id' CANNOT BE NULL");

        Submodel submodel(id);
        
        if (payload.containsKey("category"))
        {
            submodel.SetCategory(payload["category"].as<const char*>());
        }

        if (payload.containsKey("idShort"))
        {
            submodel.SetIdShort(payload["idShort"].as<const char*>());
        }

        if (payload.containsKey("semanticId"))
        {
            Reference semanticId = parseSemanticId(payload["semanticId"].as<JsonObject>());
            submodel.SetSemanticID(semanticId);
        }

        // submodel.GetQualifier();
        // submodel.GetExtensionOrNull();
        // submodel.GetDataSpecification();
    }

    /**
     * @todo Need to integrate with function below to reduce duplicated codes
     *  * psram::vector<Reference> AssetAdministrationShellDeserializer::parseReferenceToSubmodels(const JsonArray payload)
     */
    Reference SubmodelsDeserializer::parseSemanticId(const JsonObject payload)
    {
        const char* refType = payload["type"].as<const char*>();
        ASSERT((refType != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");

        const reference_types_e referenceType = strcmp(refType, "ModelReference") == 0
            ? reference_types_e::MODEL_REFERENCE
            : reference_types_e::EXTERNAL_REFERENCE;

        const size_t numKeys = payload["keys"].size();
        ASSERT((numKeys > 0), "THE SIZE OF 'keys' MUST BE GREATER THAN 0");

        psram::vector<Key> keys;
        keys.reserve(numKeys);

        for (const auto& key : payload["keys"].as<JsonArray>())
        {
            ASSERT((key.containsKey("type")), "KEY 'type' CANNOT BE MISSING");
            ASSERT((key.containsKey("value")), "KEY 'value' CANNOT BE MISSING");

            const char* strType = key["type"].as<const char*>();
            ASSERT((strType != nullptr), "THE VALUE OF 'type' CANNOT BE NULL");
            const key_types_e type = ConvertToKeyType(strType);

            const char* value = key["value"].as<const char*>();
            ASSERT((value != nullptr), "THE VALUE OF 'value' CANNOT BE NULL");
            keys.emplace_back(type, value);
        }
        ASSERT((keys.empty() == false), "THE SIZE OF 'keys' CANNOT BE ZERO");
        return Reference(referenceType, keys);
    }
}}