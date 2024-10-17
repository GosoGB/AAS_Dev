#include <Arduino.h>
#include <Jarvis/Config/Network/CatM1.h>
#include <Jarvis/Config/Interfaces/Rs232.h>
#include <Jarvis/Validators/Validator.h>

#include <DataFormat/JSON/JSON.h>
#include <Network/CatM1/CatM1.h>


#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Network/TypeDefinitions.h>
#include <Protocol/MQTT/CatMQTT/CatMQTT.h>
#include <Storage/ESP32FS/ESP32FS.h>
#include <vector>
#include <ArduinoJson.h>
#include "Jarvis/Validators/Network/NetworkValidator.h"
#include "Jarvis/Validators/Network/LteValidator.h"
#include "Jarvis/Validators/Protocol/ModbusValidator.h"
#include "Jarvis/Validators/Information/NodeValidator.h"
#include "Jarvis/Validators/Information/AlarmValidator.h"
#include "Jarvis/Validators/Information/OperationTimeValidator.h"
#include "Jarvis/Validators/Information/ProductionInfoValidator.h"
#include "Jarvis/Validators/Operation/OperationValidator.h"



static std::string PROGMEM JARVIS_DEFAULT = R"(
{
"ver":"v1","cnt":{
"prod":[{"tot":"no11","ok":null,"ng":null}],
"optime":[{
"nodeId":"no11",
"type":1,
"crit":1,
"op":"=="}],
"op":[{
"snic":"lte",
"exp":false,
"intvPoll":1,
"intvSrv":1,
"ota":false}],
"alarm":[{
"nodeId":"no11",
"type":4,
"lcl":null,
"lclUid":null, 
"ucl":null,
"uclUid":null,
"cnd":[1,2,55]]},
{
"nodeId":"no12",
"type":1,
"lcl":36,
"lclUid":"P001", 
"ucl":13.5,
"uclUid":"P002",
"cnd":[1,2,55]]}]}})";

void setup()
{
    muffin::logger = new muffin::Logger();
    // muffin::JSON* json = new muffin::JSON();

    using namespace muffin;
    using cin_vector = std::vector<jarvis::config::Base*>;
    Serial.println();
    jarvis::OperationTimeValidator* OPvalidator = new jarvis::OperationTimeValidator();
    jarvis::AlarmValidator* Alarmvalidator = new jarvis::AlarmValidator();
    jarvis::OperationValidator* Operationvalidator = new jarvis::OperationValidator();
    jarvis::ProductionInfoValidator* ProductionInfovalidator = new jarvis::ProductionInfoValidator();

    JsonDocument doc;

    deserializeJson(doc, JARVIS_DEFAULT);
    JsonArray opArray = doc["cnt"]["optime"].as<JsonArray>();
    JsonArray alarmArray = doc["cnt"]["alarm"].as<JsonArray>();
    JsonArray OperationArray = doc["cnt"]["op"].as<JsonArray>();
    JsonArray ProductionArray = doc["cnt"]["prod"].as<JsonArray>();

    cin_vector vector;

    std::pair<jarvis::rsc_e, std::string> result = OPvalidator->Inspect(opArray, &vector);
    LOG_INFO(logger, "OP result : %s" , result.second.c_str());

    result = Alarmvalidator->Inspect(alarmArray, &vector);
    LOG_INFO(logger, "ALARM result : %s" , result.second.c_str());

    result = Operationvalidator->Inspect(OperationArray, &vector);
    LOG_INFO(logger, "Operation result : %s" , result.second.c_str());

    result = ProductionInfovalidator->Inspect(ProductionArray, &vector);
    LOG_INFO(logger, "ProductionInfo result : %s" , result.second.c_str());
    
//     jarvis::config::Rs232* rs232 = new jarvis::config::Rs232("rs232");
//     jarvis::Validator* AA = new jarvis::Validator();

//     JsonDocument doc;
//     json->Deserialize(JARVIS_DEFAULT,&doc);

//     Status ret = AA->VailidateJsonFomat(doc);
//     if(ret == muffin::Status::Code::GOOD)
//     {
//         LOG_DEBUG(logger,"GOOD!!!!!!!!");
//     }

//     muffin::jarvis::config::CatM1 config;
//     config.SetCounty("KR");
//     config.SetModel("LM5");

//     muffin::CatM1 catM1;
//     catM1.Config(&config);
//     catM1.Init();

//     while (catM1.GetState() != muffin::CatM1::state_e::SUCCEDDED_TO_START)
//     {
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }

//     while (catM1.Connect() != muffin::Status::Code::GOOD)
//     {
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }

//     /***********************************************************/

//     muffin::mqtt::BrokerInfo info("test_id_mac_modlink_l");
//     muffin::mqtt::CatMQTT mqtt(catM1, info);
//     mqtt.Init(muffin::network::lte::pdp_ctx_e::PDP_01, muffin::network::lte::ssl_ctx_e::SSL_0);
//     while (mqtt.Connect() != muffin::Status::Code::GOOD)
//     {
//         vTaskDelay(3000 / portTICK_PERIOD_MS);
//     }

//     std::vector<muffin::mqtt::Message> vec;
//     muffin::mqtt::Message message(
//         muffin::mqtt::topic_e::JARVIS_READ_PARAMETER,
//         ""
//     );
//     vec.emplace_back(message);
//     mqtt.Subscribe(vec);

//     /***********************************************************/

//     muffin::http::CatHTTP http(catM1);
//     LOG_INFO(muffin::logger, "HTTP Init: %s",
//         http.Init(muffin::network::lte::pdp_ctx_e::PDP_01,
//                   muffin::network::lte::ssl_ctx_e::SSL_0
//         ).c_str()
//     );

//     muffin::http::HttpHeader header1(
//         muffin::rest_method_e::GET,
//         muffin::http_scheme_e::HTTP,
//         "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
//         5000,
//         "/api/server/time",
//         "MODLINK-L/0.0.1"
//     );
//     http.GET(header1);

//     muffin::http::HttpHeader header2(
//         muffin::rest_method_e::POST,
//         muffin::http_scheme_e::HTTP,
//         "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
//         5000,
//         "/api/firmware/version",
//         "MODLINK-L/0.0.1"
//     );

//     muffin::http::HttpBody body("application/x-www-form-urlencoded");
//     body.AddProperty("cp", "1000");
//     body.AddProperty("md", "TEST");
//     http.POST(header2, body);
//     delay(UINT32_MAX);
}

void loop()
{
    // put your main code here, to run repeatedly:
}