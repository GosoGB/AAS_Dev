/**
 * @file ModbusValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class ModbusValidator
    {
    public:
        ModbusValidator();
        virtual ~ModbusValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        std::pair<rsc_e, if_e> convertToEthernetInterfaces(uint8_t eths);
        std::pair<rsc_e, std::string> validateModbusRTU(const JsonArray array, cin_vector* outVector);
        std::pair<rsc_e, std::string> validateModbusTCP(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeysModbusRTU(const JsonObject json);
        rsc_e validateMandatoryValuesModbusRTU(const JsonObject json);
        rsc_e validateMandatoryKeysModbusTCP(const JsonObject json);
        rsc_e validateMandatoryValuesModbusTCP(const JsonObject json);
    private:
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, IPAddress> convertToIPv4(const std::string ip);
        std::pair<rsc_e, nic_e> convertToIface(const std::string iface);
        std::pair<rsc_e, std::vector<std::string>> convertToNodes(const JsonArray nodes);
        std::pair<rsc_e, prt_e> convertToPortIndex(const uint8_t portIndex);
        std::pair<rsc_e, uint8_t> convertToSlaveID(const uint8_t slaveID);
    };
}}
