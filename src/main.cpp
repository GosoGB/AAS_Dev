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
#include <IM/Node/Include/NumericAddressRange.h>



void setup()
{
    muffin::logger = new muffin::Logger();

    muffin::im::Node node1("node1", "uid1", "pid1", muffin::im::data_type_e::INT16);
    muffin::im::Node node2("node2", "uid2", "pid2", muffin::im::data_type_e::INT16);
    muffin::im::Node node3("node3", "uid3", "pid3", muffin::im::data_type_e::INT16);
    muffin::im::Node node4("node4", "uid4", "pid4", muffin::im::data_type_e::INT16);

    node1.VariableNode.SetAddress(0);
    node1.VariableNode.SetQuantity(1);
    node1.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    node2.VariableNode.SetAddress(1);
    node2.VariableNode.SetQuantity(3);
    node2.VariableNode.SetModbusArea(muffin::modbus::area_e::COIL);

    node3.VariableNode.SetAddress(5);
    node3.VariableNode.SetQuantity(3);
    node3.VariableNode.SetModbusArea(muffin::modbus::area_e::HOLDING_REGISTER);

    node4.VariableNode.SetAddress(8);
    node4.VariableNode.SetQuantity(3);
    node4.VariableNode.SetModbusArea(muffin::modbus::area_e::HOLDING_REGISTER);

    muffin::ModbusRTU mbRTU;
    mbRTU.AddNodeReference(1, node1);
    mbRTU.AddNodeReference(2, node2);
    mbRTU.AddNodeReference(1, node3);
    mbRTU.AddNodeReference(2, node4);

    // mbRTU.RemoveReferece(node3.GetNodeID());
    // mbRTU.RemoveReferece(node2.GetNodeID());
    // mbRTU.RemoveReferece(node1.GetNodeID());
    // mbRTU.RemoveReferece(node3.GetNodeID());
    
    
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