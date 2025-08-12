/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-07-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include <HardwareSerial.h>

#include <IEC63278/Container/include/AASXLoader.hpp>
#include <IEC63278/Container/Container.hpp>



void log_psram()
{
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_psram  = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_psram_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

    ESP_LOGI("PSRAM", "Total PSRAM: %d bytes", total_psram);
    ESP_LOGI("PSRAM", "Free PSRAM: %d bytes", free_psram);
    ESP_LOGI("PSRAM", "Largest free PSRAM block: %d bytes", largest_psram_block);
}


void setup()
{
    Serial.begin(115200);

    using namespace muffin;
    using namespace aas;

    log_psram();

    AASXLoader aasxLoader;
    aasxLoader.Start();

    log_psram();

    Container* container = Container::GetInstance();
    const AssetAdministrationShell* aas = container->GetAssetAdministrationShell();


    AssetInformation assetInformation = aas->GetAssetInformation();
    log_d("Asset Kind: %d", static_cast<int>(assetInformation.GetAssetKind()));
    log_d("Global Asset ID: %s", assetInformation.GetGlobalAssetID());

    psram::vector<Reference> submodels = aas->GetSubmodel();
    for (const auto& submodel : submodels)
    {
        log_d("Submodel Type: %d", static_cast<int>(submodel.GetType()));
        for (const auto& key : submodel.GetKeys())
        {
            log_d("Key Type: %s, Value: %s", 
                KEY_TYPES_STRING[static_cast<uint8_t>(key.GetType())], 
                key.GetValue().c_str());
        }
    }
}


void loop()
{
}