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
#include <Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h>
#include <Protocol/Modbus/Include/ArduinoRS485/src/RS485.h>
#include <Protocol/Modbus/Include/ArduinoModbus/src/ModbusRTUClient.h>
#include <Storage/ESP32FS/ESP32FS.h>
#include <vector>
#include <ArduinoJson.h>
#include "Jarvis/Validators/Validator.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "IM/Node/Node.h"
#include "IM/Node/Variable.h"




static std::string PROGMEM JARVIS_DEFAULT = R"({"ver":"v1","cnt":{"rs232":[],"rs485":[{"prt":2,"bdr":9600,"dbit":8,"pbit":0,"sbit":1}],"wifi":[],"eth":[],"catm1":[{"md":"LM5","ctry":"KR"}],"mbrtu":[{"prt":3,"sid":1,"nodes":["no01","no02"]}],"mbtcp":[],"op":[],"node":[{"id":"no01","adtp":0,"addr":322,"area":1,"bit":null,"qty":null,"scl":null,"ofst":null,"map":null,"ord":null,"dt":[0],"fmt":null,"uid":"DI01","name":"테스트","unit":"N/A","event":false},{"id":"no02","adtp":0,"addr":0,"area":3,"bit":null,"qty":1,"scl":-1,"ofst":null,"map":null,"ord":null,"dt":[4],"fmt":null,"uid":"DI02","name":"테스트2","unit":"N/A","event":false}],"alarm":[],"optime":[{"nodeId":"no11","type":2,"crit":1,"op":"=="}],"prod":[]}})";

void setup()
{
    muffin::logger = new muffin::Logger();
    // muffin::JSON* json = new muffin::JSON();

    using namespace muffin;
    // using cin_vector = std::vector<jarvis::config::Base*>;
    using namespace im;
    Serial.println();
    // jarvis::Validator* validator = new jarvis::Validator();
    
    // JsonDocument doc;

    // deserializeJson(doc, JARVIS_DEFAULT);

    // std::map<jarvis::cfg_key_e, cin_vector> vector;

    // jarvis::ValidationResult result = validator->Inspect(doc, &vector);
    // LOG_INFO(logger, "OP result : %s" , result.GetDescription().c_str());



    ModbusRTU modbus;   

    modbus.SetPort(Serial2);
    modbus.InitTest();
    Node node1("node11","DO01","P301",data_type_e::BOOLEAN);
    node1.VariableNode.SetAddress(0);
    node1.VariableNode.SetQuantity(1);
    node1.VariableNode.SetModbusArea(modbus::area_e::COIL);

    Node node2("node12","DO02","P302",data_type_e::BOOLEAN);
    node2.VariableNode.SetAddress(1);
    node2.VariableNode.SetQuantity(1);
    node2.VariableNode.SetModbusArea(modbus::area_e::COIL);

    Node node3("node13","DO03","P302",data_type_e::BOOLEAN);
    node3.VariableNode.SetAddress(3);
    node3.VariableNode.SetQuantity(1);
    node3.VariableNode.SetModbusArea(modbus::area_e::COIL);
    
    modbus.AddNodeReference(1,node1);
    modbus.AddNodeReference(1,node2);
    modbus.AddNodeReference(1,node3);
    
    while (true)
    {
        delay(1000);
        Status ret = modbus.Poll();
        LOG_INFO(logger," ret : %s", ret.ToString().c_str());
    }
    
    
    
    

}

void loop()
{
    // put your main code here, to run repeatedly:
}