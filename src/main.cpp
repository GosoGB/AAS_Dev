/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크가 적용된 펌웨어의 진입점입니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




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

#include <MUFFIN.h>


static std::string PROGMEM JARVIS_DEFAULT = R"({"ver":"v1","cnt":{"rs232":[],"rs485":[{"prt":2,"bdr":9600,"dbit":8,"pbit":0,"sbit":1}],"wifi":[],"eth":[],"catm1":[{"md":"LM5","ctry":"KR"}],"mbrtu":[{"prt":3,"sid":1,"nodes":["no01","no02","no03"]}],"mbtcp":[],"op":[],"node":[{"id":"no01","adtp":0,"addr":0,"area":2,"bit":null,"qty":null,"scl":null,"ofst":null,"map":null,"ord":null,"dt":[0],"fmt":null,"uid":"DI01","name":"테스트","unit":"N/A","event":false},{"id":"no02","adtp":0,"addr":1,"area":2,"bit":null,"qty":null,"scl":null,"ofst":null,"map":null,"ord":null,"dt":[0],"fmt":null,"uid":"DI02","name":"테스트2","unit":"N/A","event":false},{"id":"no03","adtp":0,"addr":3,"area":2,"bit":null,"qty":null,"scl":null,"ofst":null,"map":null,"ord":null,"dt":[0],"fmt":null,"uid":"DI03","name":"테스트3","unit":"N/A","event":false}],"alarm":[],"optime":[{"nodeId":"no11","type":2,"crit":1,"op":"=="}],"prod":[]}})";

void setup()
{
    MUFFIN muffin;
    // muffin.Start();
    muffin::logger = new muffin::Logger();
    muffin::JSON* json = new muffin::JSON();

    using namespace muffin;
    using cin_vector = std::vector<jarvis::config::Base*>;
    using namespace im;
    Serial.println();
    jarvis::Validator* validator = new jarvis::Validator();
    
    JsonDocument doc;

    deserializeJson(doc, JARVIS_DEFAULT);

    std::map<jarvis::cfg_key_e, cin_vector> vector;

    jarvis::ValidationResult result = validator->Inspect(doc, &vector);
    LOG_INFO(logger, "result : %s" , result.GetDescription().c_str());


    ModbusRTU modbus;
    Serial2.begin(9600);
   

    modbus.SetPort(Serial2);
    
    for (const auto& pair : vector) 
    {
        if (pair.first == jarvis::cfg_key_e::NODE) 
        {
            for (const auto& nodeVector : pair.second)
            {
                jarvis::config::Node* nodeConfig = static_cast<jarvis::config::Node*>(nodeVector);

                Node node(nodeConfig);
                modbus.AddNodeReference(1,node);
            }
            
        }
    }
        
    
    while (true)
    {
        delay(1000);
        Status ret = modbus.Poll();

    }

}

void loop()
{
    /**
     * @brief Arduino ESP32 Core 프레임워크의 "main.cpp" 파일에 정의된 "loopTask"를 정지합니다.
     */
    vTaskDelete(NULL);

    /**
     * @todo "HardwareSerial Serial(0)" 포트의 RxD로 데이터를 쓸 때, ESP32에서의 처리를 구현해야 합니다.
     * @details "loopTask" 내부에는 "HardwareSerial Serial(0)" 포트로 데이터가 들어오면 비록 비어있긴
     *          해도 "serialEvent(void)" 함수를 호출하고 있습니다. 
     */
    // put your main code here, to run repeatedly:
}