/**
 * @file HelperFunctions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>

#include "../../Include/Converter.hpp"
#include "../../Metamodel/Reference.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    JsonDocument ConvertToJSON(const Reference& reference)
    {
        JsonDocument doc;
        doc["type"] = ConvertToString(reference.GetType());

        const Reference* referredSemanticID = reference.GetReferredSemanticID();
        log_d("referredSemanticID: %p", referredSemanticID);
        if (referredSemanticID != nullptr)
        {
            doc["referredSemanticId"] = ConvertToJSON(*referredSemanticID);
        }

        JsonArray arrKeys = doc["keys"].to<JsonArray>();
        psram::vector<Key> keys = reference.GetKeys();
        for (size_t idx = 0; idx < keys.size(); ++idx)
        {
            JsonObject key = arrKeys.add<JsonObject>();
            key["type"] =  ConvertToString(keys[idx].GetType());
            key["value"] = keys[idx].GetValue();
        }

        return doc;
    }
    

    psram::string SerializeReference(const Reference& reference)
    {
        psram::string output;
        serializeJson(ConvertToJSON(reference), output);
        return output;
    }

    Reference DeserializeReference(const JsonObject reference)
    {
        const char* type = reference["type"].as<const char*>();
        ASSERT((type != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");
        
        const reference_types_e referenceType = strcmp(type, "ModelReference") == 0
            ? reference_types_e::MODEL_REFERENCE
            : reference_types_e::EXTERNAL_REFERENCE;

        const size_t numKeys = reference["keys"].size();
        ASSERT((numKeys > 0), "THE SIZE OF 'keys' MUST BE GREATER THAN 0");

        psram::vector<Key> keys;
        keys.reserve(numKeys);

        for (const auto& key : reference["keys"].as<JsonArray>())
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
        Reference _reference(referenceType, std::move(keys));

        if (reference.containsKey("referredSemanticId"))
        {
            _reference.SetReferredSemanticID(
                psram::make_unique<Reference>(
                    DeserializeReference(reference["referredSemanticId"].as<JsonObject>())
                )
            );
        }
        
        return _reference;
    }

}}