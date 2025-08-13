/**
 * @file AssetAdministrationShellSerializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../Include/AssetAdministrationShellSerializer.hpp"
#include "../../Container/Container.hpp"
#include "../../Include/Converter.hpp"



namespace muffin { namespace aas {


    psram::string AssetAdministrationShellSerializer::Encode(const Identifier& id)
    {
        Container* container = Container::GetInstance();
        const AssetAdministrationShell* aas = container->GetAasByID(id);
        if (aas == nullptr)
        {
            log_d("AAS Not found with shortId: '%s'", id.c_str());
            return psram::string();
        }

        psram::string output;
        JsonDocument doc = encode(*aas);
        serializeJson(doc, output);
        return output;
    }


    psram::string AssetAdministrationShellSerializer::EncodeAll()
    {
        Container* container = Container::GetInstance();
        const auto& vectorAAS = container->GetAllAAS();

        log_d("vector size: %u", vectorAAS.size());
        log_d("identifier: %s", vectorAAS[0].GetID().c_str());

        JsonDocument doc;
        JsonArray arrayAAS = doc.to<JsonArray>();

        for (const auto& aas : vectorAAS)
        {
            JsonDocument docAAS = encode(aas); // Dereference the unique_ptr
            arrayAAS.add(docAAS);
        }

        psram::string output;
        serializeJson(doc, output);
        return output;
    }


    JsonDocument AssetAdministrationShellSerializer::encode(const AssetAdministrationShell& aas)
    {
        JsonDocument doc;
        doc["modelType"] = "AssetAdministrationShell";
        doc["id"] = aas.GetID();

        if (aas.GetIdShortOrNull() != nullptr)
        {
            doc["idShort"] = aas.GetIdShortOrNull();
        }

        const AssetInformation assetInformation = aas.GetAssetInformation();
        JsonObject objAssetInformation = doc["assetInformation"].to<JsonObject>();
        objAssetInformation["assetKind"] = ConvertToString(assetInformation.GetAssetKind());

        const Reference* globalAssetId = assetInformation.GetGlobalAssetIdOrNULL();
        if (globalAssetId != nullptr)
        {
            psram::vector<Key> keys = globalAssetId->GetKeys();
            for (const auto& key : keys)
            {
                if (key.GetType() != key_types_e::GLOBAL_REFERENCE)
                {
                    continue;
                }

                objAssetInformation["globalAssetId"] = key.GetValue();
            }
        }
        
        assetInformation.GetSpecificAssetIdOrNULL();

        const psram::vector<Reference> submodelReferences = aas.GetSubmodel();
        if (submodelReferences.size() > 0)
        {
            JsonArray submodelArray = doc.createNestedArray("submodels");
            for (const auto& reference : submodelReferences)
            {
                JsonObject objSubmodel = submodelArray.createNestedObject();
                objSubmodel["type"] = ConvertToString(reference.GetType());
                
                JsonArray arrKeys = objSubmodel.createNestedArray("keys");
                psram::vector<Key> keys = reference.GetKeys();
                for (size_t idx = 0; idx < keys.size(); ++idx)
                {
                    JsonObject objKey = arrKeys.createNestedObject();
                    objKey["type"] =  ConvertToString(keys[idx].GetType());
                    objKey["value"] = keys[idx].GetValue();
                }
            }
        }

        return doc;
    }
}}