#include <Common/Logger/Logger.h>
#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Network/CatM1/Processor.h>
#include <Network/CatM1/CatM1.h>
#include <storage/ESP32FS/ESP32FS.h>

#include <Common/DataStructure/CircularBuffer.h>
#include <Arduino.h>


void setup()
{
    muffin::logger = new muffin::Logger();

    muffin::jarvis::config::CatM1 config;
    config.SetCounty("KR");
    config.SetModel("LM5");

    muffin::CatM1 catM1;
    catM1.Config(&config);
    catM1.Init();
    // while (catM1.GetState() != muffin::CatM1::state_e::SUCCEDDED_TO_START)
    // {
    //     vTaskDelay(100 / portTICK_PERIOD_MS);
    // }
    // while (catM1.Connect() != muffin::Status::Code::GOOD)
    // {
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);
    // }

    LOG_ERROR(muffin::logger, "------------------");
    LOG_ERROR(muffin::logger, " END OF SETUP());");
    LOG_ERROR(muffin::logger, "------------------");
    delay(UINT32_MAX);
}

void loop()
{
}