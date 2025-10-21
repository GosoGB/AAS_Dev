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




#include <MUFFIN.h>
#include <IEC63278/Client/Client.hpp>



void setup()
{
    MUFFIN muffin;
    muffin.Start();

    muffin::aas::Client* client = muffin::aas::Client::GetInstance();
    
    client->AddEntry("n018", "CurrentPositionMm");
    client->AddEntry("n017", "CurrentPositionYd");
    client->AddEntry("n016", "LengthOffsetMm");
    client->AddEntry("n015", "LengthSettingYd");
    client->AddEntry("n014", "CurrentOutputEA");
    client->AddEntry("n013", "TargetQuantityEA");
    client->AddEntry("n012", "TensionCompStartPct");
    client->AddEntry("n011", "TensionCompMidPct");
    client->AddEntry("n010", "TensionCompEndPct");
    client->AddEntry("n009", "Roller1ManualRatioPct");
    client->AddEntry("n008", "Roller2ManualRatioPct");
    client->AddEntry("n007", "Roller1AutoRatioPct");
    client->AddEntry("n006", "Roller2AutoRatioPct");
    client->AddEntry("n005", "ElevatorRiseTimeSec");
    client->AddEntry("n004", "CutLimitDelaySec");
    client->AddEntry("n003", "CutterWheelAutoSpeedRpm");
    client->AddEntry("n002", "WebDetectDelaySec");
    client->AddEntry("n001", "TensionCompStopPct");

    client->Start();
    
    // AssetAdministrationShellSerializer serializer;
    // psram::string json = serializer.Encode("https://example.com/ids/sm/4523_3042_7052_7604");
    // log_d("/shells/{{identifier}}\n%s\n", json.c_str());
    // Serial.println();

    // json = serializer.EncodeAll();
    // log_d("/shells\n%s\n", json.c_str());
    // Serial.println();

    
    // json = submodelSerializer.EncodeAll();
    // log_d("/submodels\n%s\n", json.c_str());
    // Serial.println();

    // log_psram();
}


void loop()
{
}