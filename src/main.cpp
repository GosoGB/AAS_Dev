#include <Arduino.h>
#include <IM/Node/include/NodeID.h>
#include <IM/Node/Base.h>
#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Storage/ESP32FS/ESP32FS.h>
#include <map>



void setup()
{
    Serial.begin(115200);

    using namespace muffin;

    std::string message = "Hello, World! I am a Node #02!! Nice to meet you!";
    im::NodeID node01(0, 123123);
    im::NodeID node02(0, message);

    im::Base base01(node01);
    im::Base base02(node02);
}

void loop()
{
    // put your main code here, to run repeatedly:
}