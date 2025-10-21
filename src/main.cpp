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
    
    client->AddEntry("n001", "CurrentPositionMm");
    client->AddEntry("n002", "CurrentPositionYd");
    client->AddEntry("n003", "LengthOffsetMm");
    client->AddEntry("n004", "LengthSettingYd");
    client->AddEntry("n005", "CurrentOutputEA");
    client->AddEntry("n006", "TargetQuantityEA");
    client->AddEntry("n007", "TensionCompStartPct");
    client->AddEntry("n008", "TensionCompMidPct");
    client->AddEntry("n009", "TensionCompEndPct");
    client->AddEntry("n010", "Roller1ManualRatioPct");
    client->AddEntry("n011", "Roller2ManualRatioPct");
    client->AddEntry("n012", "Roller1AutoRatioPct");
    client->AddEntry("n013", "Roller2AutoRatioPct");
    client->AddEntry("n014", "ElevatorRiseTimeSec");
    client->AddEntry("n015", "CutLimitDelaySec");
    client->AddEntry("n016", "CutterWheelAutoSpeedRpm");
    client->AddEntry("n017", "WebDetectDelaySec");
    client->AddEntry("n018", "TensionCompStopPct");

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