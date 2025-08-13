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
#include <IEC63278/Serializer/Serializer.hpp>



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

    AssetAdministrationShellSerializer serializer;

    psram::string json = serializer.Encode("https://example.com/ids/sm/4523_3042_7052_7604");
    log_d("%s", json.c_str());
    Serial.println();

    json = serializer.EncodeAll();
    log_d("%s", json.c_str());
    Serial.println();

    log_psram();
}


void loop()
{
}