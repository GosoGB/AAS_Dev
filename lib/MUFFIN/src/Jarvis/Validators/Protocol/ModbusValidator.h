/**
 * @file ModbusValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class ModbusValidator
    {
    public:
        ModbusValidator();
        virtual ~ModbusValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateModbusRTU(const JsonArray array, cin_vector* outVector);
        Status validateModbusTCP(const JsonArray array, cin_vector* outVector);
        Status validateMandatoryKeysModbusRTU(const JsonObject json);
        Status validateMandatoryValuesModbusRTU(const JsonObject json);
        Status validateMandatoryKeysModbusTCP(const JsonObject json);
        Status validateMandatoryValuesModbusTCP(const JsonObject json);
    private:
        Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<Status, IPAddress> convertToIPv4(const std::string ip);
        std::pair<Status, nic_e> convertToIface(const std::string iface);
        std::pair<Status, std::vector<std::string>> convertToNodes(const JsonArray nodes);
        std::pair<Status, prt_e> convertToPortIndex(const uint8_t portIndex);
        std::pair<Status, uint8_t> convertToSlaveID(const uint8_t slaveID);
    };
}}
