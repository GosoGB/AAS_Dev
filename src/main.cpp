#include <Arduino.h>
// #include <Jarvis/Config/Network/CatM1.h>
// #include <Jarvis/Config/Interfaces/Rs232.h>
// #include <Jarvis/Validators/Validator.h>

// #include <DataFormat/JSON/JSON.h>
// #include <Network/CatM1/CatM1.h>


// #include <Network/Ethernet/Ethernet.h>
// #include <Network/WiFi4/WiFi4.h>
// #include <Network/TypeDefinitions.h>
// #include <Protocol/MQTT/CatMQTT/CatMQTT.h>
// #include <Storage/ESP32FS/ESP32FS.h>
// #include <vector>
// #include <ArduinoJson.h>
#include <Jarvis/Validators/Interfaces/SerialPortValidator.h>



// static std::string PROGMEM JARVIS_DEFAULT = R"({"ver":"v1","cnt":{"rs232":[],"rs485":[{"prt":2,"bdr":9600,"dbit":8,"pbit":0,"sbit":1}],"wifi":[],"eth":[],"catm1":[{"md":"LM5","ctry":"KR"}],"mbrtu":[{"ptr":2,"sid":1,"nodes":["no01","no02"]}],"mbtcp":[],"op":[{"snic":"lte","exp":true,"intvPoll":1,"intvSrv":60,"ota":false}],"node":[{"id":"no01","adtp":0,"addr":0,"area":4,"bit":null,"qty":1,"scl":-1,"ofst":null,"dt":3,"map":null,"ord":null,"uid":"DI1","fmt":null,"name":"테스트","unit":"N/A","event":true},{"id":"no01","adtp":0,"addr":1,"area":4,"bit":3,"qty":null,"scl":null,"ofst":null,"dt":3,"map":{"0":"닫힘","1":"열림"},"ord":"nuill","uid":"DO1","name":"테스트","unit":"N/A","event":true}],"alarm":[],"optime":[],"prod":[]}})";

void setup()
{
//     muffin::logger = new muffin::Logger();
//     muffin::JSON* json = new muffin::JSON();

//     using namespace muffin;
//     Serial.println();
    
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