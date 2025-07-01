/**
 * @file EthernetIpValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
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

    class EthernetIpValidator
    {
    public:
        EthernetIpValidator();
        virtual ~EthernetIpValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        std::pair<rsc_e, std::string> validateEthernetIP(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeysEthernetIP(const JsonObject json);
        rsc_e validateMandatoryValuesEthernetIP(const JsonObject json);
    private:
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, if_e> convertToEthernetInterfaces(uint8_t eths);
        std::pair<rsc_e, IPAddress> convertToIPv4(const std::string ip);
        std::pair<rsc_e, std::vector<std::string>> convertToNodes(const JsonArray nodes);

    };
}}
