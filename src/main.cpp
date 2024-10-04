#include <Arduino.h>
#include <IM/Node/Include/NodeID.h>
#include <IM/Node/Include/Helper.h>
#include <IM/Node/Node.h>
#include <Network/Ethernet/Ethernet.h>
#include <Network/WiFi4/WiFi4.h>
#include <Storage/ESP32FS/ESP32FS.h>
#include <map>

#include <Protocol/Modbus/ModbusRTU.h>
#include <Protocol/Modbus/Include/TypeDefinitions.h>
#include <Protocol/Modbus/Include/Address.h>
#include <IM/Node/Include/NumericAddressRange.h>



void setup()
{
    muffin::logger = new muffin::Logger();

    muffin::im::Node node1("node1", "uid1", "pid1", muffin::im::data_type_e::BOOLEAN);
    muffin::im::Node node2("node2", "uid2", "pid2", muffin::im::data_type_e::BOOLEAN);
    muffin::im::Node node3("node3", "uid3", "pid3", muffin::im::data_type_e::BOOLEAN);
    muffin::im::Node node4("node4", "uid4", "pid4", muffin::im::data_type_e::BOOLEAN);

    node1.VariableNode.SetAddress(0);
    node1.VariableNode.SetQuantity(1);
    node1.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    node2.VariableNode.SetAddress(1);
    node2.VariableNode.SetQuantity(1);
    node2.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    node3.VariableNode.SetAddress(2);
    node3.VariableNode.SetQuantity(1);
    node3.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    node4.VariableNode.SetAddress(3);
    node4.VariableNode.SetQuantity(1);
    node4.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    muffin::ModbusRTU mbRTU;
    mbRTU.AddNodeReference(1, node1);
    mbRTU.AddNodeReference(1, node2);
    mbRTU.AddNodeReference(1, node3);
    mbRTU.AddNodeReference(1, node4);

    while (true)
    {
        const uint32_t startMillis = millis();
        mbRTU.Poll();
        LOG_DEBUG(muffin::logger, "Processing time: %u", millis() - startMillis);

        LOG_DEBUG(muffin::logger, "node #1: %s", node1.VariableNode.RetrieveData().Value.Boolean ? "true" : "false");
        LOG_DEBUG(muffin::logger, "node #2: %s", node2.VariableNode.RetrieveData().Value.Boolean ? "true" : "false");
        LOG_DEBUG(muffin::logger, "node #3: %s", node3.VariableNode.RetrieveData().Value.Boolean ? "true" : "false");
        LOG_DEBUG(muffin::logger, "node #4: %s", node4.VariableNode.RetrieveData().Value.Boolean ? "true" : "false");
        delay(1000);
    }
}

void loop()
{
    // put your main code here, to run repeatedly:
}