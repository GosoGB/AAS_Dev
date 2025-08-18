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
#include "../Include/HelperFunctions.hpp"
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
            Reference semanticId = DeserializeReference(payload["semanticId"].as<JsonObject>());
            submodel.SetSemanticID(semanticId);
        }

        // submodel.GetQualifier();
        // submodel.GetExtensionOrNull();
        // submodel.GetDataSpecificationOrNULL();
    }
}}