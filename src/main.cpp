#include <Arduino.h>
#include <Jarvis/Config/Network/CatM1.h>
#include <Network/CatM1/CatM1.h>
#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Network/TypeDefinitions.h>
#include <Protocol/HTTP/HttpHeader.h>
#include <Protocol/HTTP/CatHTTP/CatHTTP.h>
#include <Protocol/MQTT/CatMQTT/CatMQTT.h>
#include <Storage/ESP32FS/ESP32FS.h>

#include <Protocol/HTTP/HttpResponse.h>
#include <Protocol/HTTP/HttpQueue.h>
#include <vector>


void setup()
{
    muffin::logger = new muffin::Logger();

    using namespace muffin;
    std::vector<uint8_t> buffer = { 0, 1, 2, 3, 4 };
    HttpResponse<std::vector<uint8_t>> response(buffer);

    HttpQueue queue;
    LOG_DEBUG(logger, "Occupied: %u", queue.CountOccupied());
    queue.Send(response);
    LOG_DEBUG(logger, "Occupied: %u", queue.CountOccupied());

    HttpResponse<uint8_t>* retrievedBuffer = queue.Peek();
    for (const auto& datum : retrievedBuffer)
    {
        Serial.print(datum);
        Serial.print(" ");
    }
    Serial.println();
    

    // muffin::jarvis::config::CatM1 config;
    // config.SetCounty("KR");
    // config.SetModel("LM5");

    // muffin::CatM1 catM1;
    // catM1.Config(&config);
    // catM1.Init();

    // while (catM1.GetState() != muffin::CatM1::state_e::SUCCEDDED_TO_START)
    // {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // while (catM1.Connect() != muffin::Status::Code::GOOD)
    // {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // /***********************************************************/

    // muffin::mqtt::BrokerInfo info("test_id_mac_modlink_l");
    // muffin::mqtt::CatMQTT mqtt(catM1, info);
    // mqtt.Init(muffin::network::lte::pdp_ctx_e::PDP_01, muffin::network::lte::ssl_ctx_e::SSL_0);
    // while (mqtt.Connect() != muffin::Status::Code::GOOD)
    // {
    //     vTaskDelay(3000 / portTICK_PERIOD_MS);
    // }

    // std::vector<muffin::mqtt::Message> vec;
    // muffin::mqtt::Message message(
    //     muffin::mqtt::topic_e::JARVIS_READ_PARAMETER,
    //     ""
    // );
    // vec.emplace_back(message);
    // mqtt.Subscribe(vec);

    // /***********************************************************/

    // muffin::http::CatHTTP http(catM1);
    // LOG_INFO(muffin::logger, "HTTP Init: %s",
    //     http.Init(muffin::network::lte::pdp_ctx_e::PDP_01,
    //               muffin::network::lte::ssl_ctx_e::SSL_0
    //     ).c_str()
    // );

    // muffin::http::HttpHeader header1(
    //     muffin::rest_method_e::GET,
    //     muffin::http_scheme_e::HTTP,
    //     "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
    //     5000,
    //     "/api/server/time",
    //     "MODLINK-L/0.0.1"
    // );
    // http.GET(header1);

    // muffin::http::HttpHeader header2(
    //     muffin::rest_method_e::POST,
    //     muffin::http_scheme_e::HTTP,
    //     "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
    //     5000,
    //     "/api/firmware/version",
    //     "MODLINK-L/0.0.1"
    // );

    // muffin::http::HttpBody body("application/x-www-form-urlencoded");
    // body.AddProperty("cp", "1000");
    // body.AddProperty("md", "TEST");
    // http.POST(header2, body);
    // delay(UINT32_MAX);
}

void loop()
{
    // put your main code here, to run repeatedly:
}