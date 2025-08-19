/**
 * @file AASXLoader.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include <esp32-hal-log.h>

#include "../../Metamodel/Abstract/Submodel/Submodel.hpp"
#include "../../Metamodel/AssetAdministrationShell.hpp"
#include "../../Serializer/Deserializer.hpp"
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
        
        DeserializationError error = deserializeJson(mJsonDocument, CONVERTED_AASX_FILE);
        if (error)
        {
            log_d("Failed to deserialize AASX JSON file: %s", error.c_str());
            return;
        }
        
        loadAssetAdministrationShells();
        loadSubmodels();
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


    void AASXLoader::loadAssetAdministrationShells()
    {
        ASSERT((mJsonDocument.containsKey("assetAdministrationShells")), 
            "MISSING MANDATORY ATTRIBUTE 'assetAdministrationShells'");
        
        JsonArray arrayAssetAdministrationShells = mJsonDocument["assetAdministrationShells"].as<JsonArray>();
        log_d("Size of 'assetAdministrationShells': %u", arrayAssetAdministrationShells.size());

        for (size_t idx = 0; idx < arrayAssetAdministrationShells.size(); ++idx)
        {
            JsonObject objectAssetAdministrationShell = arrayAssetAdministrationShells[idx].as<JsonObject>();

            AssetAdministrationShellDeserializer deserializer;
            psram::unique_ptr<AssetAdministrationShell> aas = deserializer.Parse(objectAssetAdministrationShell);
        #ifndef DEBUG
            구현 필요함 --> AssetAdministrationShell::SetCategory()
            구현 필요함 --> AssetAdministrationShell::SetDataSpecification()
            구현 필요함 --> AssetAdministrationShell::SetDerivedFrom()
            구현 필요함 --> AssetAdministrationShell::SetExtension()
        #endif
            Container* container = Container::GetInstance();
            container->AddAssetAdministrationShell(std::move(aas));
        }
    }


    void AASXLoader::loadSubmodels()
    {
        ASSERT((mJsonDocument.containsKey("submodels")), "MISSING MANDATORY ATTRIBUTE 'submodels'");

        JsonArray arraySubmodels = mJsonDocument["submodels"].as<JsonArray>();
        log_d("Size of 'submodels': %u", arraySubmodels.size());

        for (size_t idx = 0; idx < arraySubmodels.size(); ++idx)
        {
            JsonObject objectSubmodel = arraySubmodels[idx].as<JsonObject>();

            SubmodelsDeserializer deserializer;
            psram::unique_ptr<Submodel> submodel = deserializer.Parse(objectSubmodel);
        #ifndef DEBUG
            구현 필요함 --> AssetAdministrationShell::SetCategory()
            구현 필요함 --> AssetAdministrationShell::SetDataSpecification()
            구현 필요함 --> AssetAdministrationShell::SetDerivedFrom()
            구현 필요함 --> AssetAdministrationShell::SetExtension()
        #endif
            Container* container = Container::GetInstance();
            container->AddSubmodel(std::move(submodel));
        }
    }
}}