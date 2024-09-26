#include <Arduino.h>
#include <IM/Node/Include/NodeID.h>
#include <IM/Node/Include/Helper.h>
#include <IM/Node/Base.h>
#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Storage/ESP32FS/ESP32FS.h>
#include <map>



void setup()
{
    using namespace muffin;
    logger = new Logger();


/*
    std::string strbrowseName = "Node #01";
    im::string_t strBrowseName;

    Status ret = im::ConvertString(strbrowseName, &strBrowseName);
    if (ret != Status::Code::GOOD)
    {
        LOG_ERROR(logger, "%s", ret.c_str());
    }
    LOG_INFO(logger, "[%s] Converted: %s", ret.c_str(), ConvertString(strBrowseName).c_str());

    im::NodeID node01(0, 123123);
    im::qualified_name_t browseName01;
    browseName01.Name = strBrowseName;
    browseName01.NamespaceIndex = node01.GetNamespaceIndex();

    im::Base base01(node01, im::class_type_e::VARIABLE, browseName01);
*/
}

void loop()
{
    // put your main code here, to run repeatedly:
}