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




#include <Arduino.h>
#include "Metamodel/AssetInformation.hpp"
#include "Metamodel/Abstract/Submodel/Property.hpp"



void log(muffin::aas::asset_kind_e assetKind)
{
    using namespace muffin;
    using namespace aas;

    log_d("assetKind: %s", ASSET_KIND_STRING[static_cast<uint8_t>(assetKind)]);
}

void log(muffin::aas::reference_types_e referenceType)
{
    using namespace muffin;
    using namespace aas;

    log_d("referenceType: %s", REFERENCE_TYPES_STRING[static_cast<uint8_t>(referenceType)]);
}

void log(const muffin::psram::vector<muffin::aas::Key>& keys)
{
    using namespace muffin;
    using namespace aas;

    for (auto it = keys.begin(); it != keys.end(); ++it)
    {
        log_d("[key] {\"type\": %s, \"value\": %s}",
            KEY_TYPES_STRING[static_cast<uint8_t>(it->GetType())], 
            it->GetValue().c_str()
        );
    }
}

void log(std::weak_ptr<muffin::aas::Reference> reference)
{
    using namespace muffin;
    using namespace aas;

    if (std::shared_ptr<muffin::aas::Reference> sp = reference.lock())
    {
        log(sp->GetType());
        log(sp->GetReferredSemanticID());
        log(sp->GetKeys());
    }
    else
    {
        log_e("reference has been expired");
    }
}

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

    AssetInformation assetInformation;
    assetInformation.SetGlobalAssetID("https://example.com/ids/asset/9431_2232_7052_2169");

    log(assetInformation.GetAssetKind());
    log(assetInformation.GetGlobalAssetID()->GetType());
    log(assetInformation.GetGlobalAssetID()->GetReferredSemanticID());
    log(assetInformation.GetGlobalAssetID()->GetKeys());
    
    if (assetInformation.GetSpecificAssetID() == nullptr)
    {
        log_d("Empty specificAssetId");
    }

    Property<data_type_def_xsd_e::STRING> property;
    const data_type_def_xsd_e xsd = property.GetValueType();
    log_d("xsd: %d", xsd);

    property.SetValue("value");
    log_d("xsd value: %s", property.GetValue()->c_str());

    while (1)
    {
        psram::string _val = psram::string(std::to_string(millis()).c_str());
        property.SetValue(_val);
        log_d("xsd value: %s", property.GetValue()->c_str());
        ESP_LOGI("PSRAM", "Free PSRAM: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

        delay(999);
    }
}


void loop()
{
}