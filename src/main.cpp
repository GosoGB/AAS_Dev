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
#include <WiFi.h>

#include <IEC63278/Server/Server.hpp>

#include <Network/Ethernet/W5500/Server/Server.hpp>
#include <Network/Ethernet/W5500/W5500.h>
#include <IM/Custom/MacAddress/MacAddress.h>

#include <MUFFIN.h>

void log_psram()
{
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_psram  = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_psram_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

    ESP_LOGI("PSRAM", "Total PSRAM: %d bytes", total_psram);
    ESP_LOGI("PSRAM", "Free PSRAM: %d bytes", free_psram);
    ESP_LOGI("PSRAM", "Largest free PSRAM block: %d bytes\n\n", largest_psram_block);
}


void setup()
{
    using namespace muffin;
    using namespace muffin::w5500;

    MUFFIN muffin;
    muffin.Start();
    
    logger.Init();

    ethernet = new(std::nothrow) W5500(w5500::if_e::EMBEDDED);
    ethernet->Init();
    char mac[13] = {'\0'};
    ethernet->GetMacAddress(mac);
    macAddress.SetMacAddress(mac);

    ethernet->Connect();

    muffin::w5500::Server server;
    server.Begin(80);

    // WiFi.begin("device_team_2.4GHz", "EdgeCross123!@#");
    // // WiFi.begin("Drop It Like It's Hotspot", "RUC=M&ZfE/2hmA,:-");
    // Serial.print("Connecting");
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println();
    // Serial.print("Connected, IP address: ");
    // Serial.println(WiFi.localIP());

    // const long gmtOffset_sec = 9 * 3600;  
    // const int daylightOffset_sec = 0;     
    // const char* ntpServer = "pool.ntp.org";
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // log_psram();

    // muffin::aas::Server* server = muffin::aas::Server::GetInstance();;
    // server->Init();
        
    // using namespace muffin;
    // using namespace aas;

    // AASXLoader aasxLoader;
    // aasxLoader.Start();

    // AssetAdministrationShellSerializer serializer;
    // psram::string json = serializer.Encode("https://example.com/ids/sm/4523_3042_7052_7604");
    // log_d("/shells/{{identifier}}\n%s\n", json.c_str());
    // Serial.println();

    // json = serializer.EncodeAll();
    // log_d("/shells\n%s\n", json.c_str());
    // Serial.println();

    // SubmodelsSerializer submodelSerializer;
    // json = submodelSerializer.Encode("https://example.com/ids/sm/Identification");
    // log_d("/submodels/{{identifier}}\n%s\n", json.c_str());
    // Serial.println();
    
    // json = submodelSerializer.EncodeAll();
    // log_d("/submodels\n%s\n", json.c_str());
    // Serial.println();

    // log_psram();
}


void loop()
{
}