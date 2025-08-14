/**
 * @file HelperFunctions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-14
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


    JsonDocument ConvertReferenceToJSON(const Reference& reference);
    

    psram::string SerializeReference(const Reference& reference)
    {
        psram::string output;
        serializeJson(ConvertReferenceToJSON(reference), output);
        return output;
    }


    JsonDocument ConvertReferenceToJSON(const Reference& reference)
    {
        JsonDocument doc;
        doc["type"] = ConvertToString(reference.GetType());

        std::weak_ptr<Reference> referredSemanticID = reference.GetReferredSemanticID();
        if (referredSemanticID.expired() == false)
        {
            std::shared_ptr<Reference> spReferredSemanticID = referredSemanticID.lock();
            doc["referredSemanticId"] = SerializeReference(*spReferredSemanticID);
        }

        JsonArray arrKeys = doc["keys"].to<JsonArray>();
        psram::vector<Key> keys = reference.GetKeys();
        for (size_t idx = 0; idx < keys.size(); ++idx)
        {
            JsonObject key = arrKeys.createNestedObject();
            key["type"] =  ConvertToString(keys[idx].GetType());
            key["value"] = keys[idx].GetValue();
        }

        return doc;
    }


    Reference DeserializeReference(const Reference& reference)
    {
        ;
    }

}}