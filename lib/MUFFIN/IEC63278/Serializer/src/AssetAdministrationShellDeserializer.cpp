/**
 * @file AssetAdministrationShellDeserializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Container/Container.hpp"
#include "../../Include/Converter.hpp"
#include "../Include/AssetAdministrationShellDeserializer.hpp"

#include "Common/Assert.hpp"



namespace muffin { namespace aas {


    psram::unique_ptr<AssetAdministrationShell> AssetAdministrationShellDeserializer::Parse(const JsonObject payload)
    {
        ASSERT(("AssetAdministrationShell" == payload["modelType"]), "INVALID VALUE FOR ATTRIBUTE 'modelType'");

        ASSERT((payload.containsKey("id")), "MISSING MANDATORY ATTRIBUTE 'id'");
        const char* id = payload["id"].as<const char*>();
        ASSERT((id != nullptr), "ATTRIBUTE 'id' CANNOT BE NULL");

        ASSERT((payload.containsKey("assetInformation")), "MISSING MANDATORY ATTRIBUTE 'assetInformation'");
        const JsonObject jsonAssetInformation = payload["assetInformation"].as<JsonObject>();
        AssetInformation assetInformation = parseAssetInformation(jsonAssetInformation);

        AssetAdministrationShell aas(id, assetInformation);
        
        if (payload.containsKey("idShort"))
        {
            const char* idShort = payload["idShort"].as<const char*>();
            ASSERT((idShort != nullptr), "ATTRIBUTE 'idShort' CANNOT BE NULL");
            aas.SetIdShort(idShort);
        }

        if (payload.containsKey("submodels"))
        {
            psram::vector<Reference> submodels = parseReferenceToSubmodels(payload["submodels"].as<JsonArray>());
            ASSERT((submodels.size() > 0), "THE LENGTH OF 'submodels' MUST BE GREATER THAN 0");
            aas.SetSubmodel(std::move(submodels));
        }

        if (payload.containsKey("derivedFrom"))
        {
            ASSERT(false, "NOT IMPLEMENTED: Attribute 'derivedFrom'");
        }

        log_d("shortId: %s", aas.GetIdShortOrNull());
        return psram::make_unique<AssetAdministrationShell>(std::move(aas));
    }


    AssetInformation AssetAdministrationShellDeserializer::parseAssetInformation(const JsonObject payload)
    {
        AssetInformation assetInformation;

        ASSERT((payload.containsKey("assetKind")), "MISSING MANDATORY ATTRIBUTE 'assetKind'");
        const char* assetKind = payload["assetKind"].as<const char*>();
        ASSERT((assetKind != nullptr), "ATTRIBUTE 'assetKind' CANNOT BE NULL");
        ASSERT((
            (strcmp(assetKind, "Instance") == 0) || (strcmp(assetKind, "Type") == 0)), 
            "INVALID VALUE FOR ATTRIBUTE 'assetKind'"
        );

        if (strcmp(assetKind, "Type") == 0)
        {
            assetInformation.SetAssetKind(asset_kind_e::TYPE);
        }

        if (payload.containsKey("globalAssetId"))
        {
            const char* globalAssetId = payload["globalAssetId"].as<const char*>();
            ASSERT((globalAssetId != nullptr), "THE VALUE OF 'globalAssetId' CANNOT BE NULL");
            ASSERT((strlen(globalAssetId) > 0), "THE LENGTH OF 'globalAssetId' MUST BE GREATER THAN 0");
            assetInformation.SetGlobalAssetID(globalAssetId);
        }

        if (payload.containsKey("specificAssetId"))
        {
            ASSERT(false, "NOT IMPLEMENTED: Attribute 'specificAssetId'");
        }

        return assetInformation;
    }


    psram::vector<Reference> AssetAdministrationShellDeserializer::parseReferenceToSubmodels(const JsonArray payload)
    {
        const size_t numSubmodels = payload.size();
        ASSERT((numSubmodels > 0), "THE SIZE OF 'submodels' MUST BE GREATER THAN 0");

        psram::vector<Reference> submodels;
        submodels.reserve(numSubmodels);

        for (const auto& submodel : payload)
        {
            const char* refType = submodel["type"].as<const char*>();
            ASSERT((refType != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");

            const reference_types_e referenceType = strcmp(refType, "ModelReference") == 0
                ? reference_types_e::ModelReference
                : reference_types_e::GlobalReference;

            const size_t numKeys = submodel["keys"].size();
            ASSERT((numKeys > 0), "THE SIZE OF 'keys' MUST BE GREATER THAN 0");

            psram::vector<Key> keys;
            keys.reserve(numKeys);

            for (const auto& key : submodel["keys"].as<JsonArray>())
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

            submodels.emplace_back(referenceType, std::move(keys));
        }
        ASSERT((submodels.empty() == false), "THE SIZE OF 'submodels' CANNOT BE ZERO");
        return submodels;
    }
}}