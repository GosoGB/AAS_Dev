/**
 * @file AASXLoader.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include <esp32-hal-log.h>

#include "../../Metamodel/AssetAdministrationShell.hpp"
#include "../include/AASXLoader.hpp"
#include "../include/CONVERTED_AASX_FILE.hpp"
#include "../Container.hpp"

#include "Common/Assert.hpp"



namespace muffin { namespace aas {


    void AASXLoader::Start()
    {
    #ifndef DEBUG
        microSD 카드에서 AASX JSON 파일 읽어오기 구현해야 함
    #endif
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, CONVERTED_AASX_FILE);
        if (error)
        {
            log_d("Failed to deserialize AASX JSON file: %s", error.c_str());
            return;
        }

        JsonArray arrayShells = doc["assetAdministrationShells"].as<JsonArray>();
        for (size_t idx = 0; idx < arrayShells.size(); ++idx)
        {
            JsonObject shell = arrayShells[idx].as<JsonObject>();
            loadAssetAdministrationShells(shell);
        }
    }


    void AASXLoader::countAASX()
    {
    #ifndef DEBUG
        아직 구현 안 됨
    #endif
    }

    
    void AASXLoader::readAASX()
    {
    #ifndef DEBUG
        아직 구현 안 됨
    #endif
    }


    /**
     * @brief Converts a string representation of a key type to its corresponding enum value.
     */
    static key_types_e convertToKeyType(const char* strType)
    {
        if (strcmp(strType, "Referable") == 0)
        {
            return key_types_e::REFERABLE;
        }
        else if (strcmp(strType, "FragmentReference") == 0)
        {
            return key_types_e::FRAGMENT_REFERENCE;
        }
        else if (strcmp(strType, "GlobalReference") == 0)
        {
            return key_types_e::GLOBAL_REFERENCE;
        }
        else if (strcmp(strType, "AssetAdministrationShell") == 0)
        {
            return key_types_e::ASSET_ADMINISTRATION_SHELL;
        }
        else if (strcmp(strType, "ConceptDescription") == 0)
        {
            return key_types_e::CONCEPT_DESCRIPTION;
        }
        else if (strcmp(strType, "Identifiable") == 0)
        {
            return key_types_e::IDENTIFIABLE;
        }
        else if (strcmp(strType, "Submodel") == 0)
        {
            return key_types_e::SUBMODEL;
        }
        else if (strcmp(strType, "AnnotatedRelationshipElement") == 0)
        {
            return key_types_e::ANNOTATED_RELATIONSHIP_ELEMENT;
        }
        else if (strcmp(strType, "BasicEventElement") == 0)
        {
            return key_types_e::BASIC_EVENT_ELEMENT;
        }
        else if (strcmp(strType, "Blob") == 0)
        {
            return key_types_e::BLOB;
        }
        else if (strcmp(strType, "Capability") == 0)
        {
            return key_types_e::CAPABILITY;
        }
        else if (strcmp(strType, "DataElement") == 0)
        {
            return key_types_e::DATA_ELEMENT;
        }
        else if (strcmp(strType, "Entity") == 0)
        {
            return key_types_e::ENTITY;
        }
        else if (strcmp(strType, "EventElement") == 0)
        {
            return key_types_e::EVENT_ELEMENT;
        }
        else if (strcmp(strType, "File") == 0)
        {
            return key_types_e::FILE;
        }
        else if (strcmp(strType, "MultiLanguageProperty") == 0)
        {
            return key_types_e::MULTI_LANGUAGE_PROPERTY;
        }
        else if (strcmp(strType, "Operation") == 0)
        {
            return key_types_e::OPERATION;
        }
        else if (strcmp(strType, "Property") == 0)
        {
            return key_types_e::PROPERTY;
        }
        else if (strcmp(strType, "Range") == 0)
        {
            return key_types_e::RANGE;
        }
        else if (strcmp(strType, "ReferenceElement") == 0)
        {
            return key_types_e::REFERENCE_ELEMENT;
        }
        else if (strcmp(strType, "RelationshipElement") == 0)
        {
            return key_types_e::RELATIONSHIP_ELEMENT;
        }
        else if (strcmp(strType, "SubmodelElement") == 0)
        {
            return key_types_e::SUBMODEL_ELEMENT;
        }
        else
        {
            ASSERT(false, "UNDEFINED KEY TYPE: %s", strType);
            log_e("Unknown key type: %s", strType);
        }
    }

    
    /**
     * @note 
     * I.A.W. JSON format of AAS, JSON property representing an aggregation attribute must be 
     * omitted if the aggregation is empty. Hence, the size of the property must be greater than 0.
     */
    psram::vector<Reference> AASXLoader::parseSubmodels(const JsonArray& arraySubmodel)
    {
        const size_t numSubmodels = arraySubmodel.size();
        ASSERT((numSubmodels > 0), "THE SIZE OF 'submodels' MUST BE GREATER THAN 0");

        psram::vector<Reference> submodels;
        submodels.reserve(numSubmodels);

        for (const auto& submodel : arraySubmodel)
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
                const key_types_e type = convertToKeyType(strType);

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


    AssetInformation AASXLoader::parseAssetInformation(JsonObject assetInfo)
    {
        AssetInformation assetInformation;

        const char* assetKind = assetInfo["assetKind"].as<const char*>();
        if (strcmp(assetKind, "Type") == 0)
        {
            assetInformation.SetAssetKind(asset_kind_e::TYPE);
        }

        if (assetInfo.containsKey("globalAssetId"))
        {
            const char* globalAssetId = assetInfo["globalAssetId"].as<const char*>();
            ASSERT((globalAssetId != nullptr), "THE VALUE OF 'globalAssetId' CANNOT BE NULL");
            assetInformation.SetGlobalAssetID(globalAssetId);
        }

        return assetInformation;
    }


    void AASXLoader::loadAssetAdministrationShells(JsonObject shell)
    {
        const char* id = shell["id"].as<const char*>();
        ASSERT((id != nullptr), "ATTRIBUTE 'id' CANNOT BE NULL");
        AssetInformation assetInformation = parseAssetInformation(shell["assetInformation"]);

        auto aas = psram::make_unique<AssetAdministrationShell>(id, assetInformation);

        if (shell.containsKey("idShort"))
        {
            const char* idShort = shell["idShort"].as<const char*>();
            ASSERT((idShort != nullptr), "ATTRIBUTE 'idShort' CANNOT BE NULL");
            aas->SetIdShort(idShort);
        }
        
        if (shell.containsKey("submodels"))
        {
            psram::vector<Reference> submodels = parseSubmodels(shell["submodels"].as<JsonArray>());
            aas->SetSubmodel(std::move(submodels));
        }
        
        // aas.SetCategory
        // aas.SetDataSpecification
        // aas.SetDerivedFrom
        // aas.SetExtension

        Container* container = Container::GetInstance();
        container->AddAssetAdministrationShell(std::move(aas));
    }
}}