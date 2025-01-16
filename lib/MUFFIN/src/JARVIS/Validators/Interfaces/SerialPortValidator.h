/**
 * @file SerialPortValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-15
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



namespace muffin { namespace jarvis {

    class SerialPortValidator
    {
    public:
        SerialPortValidator();
        virtual ~SerialPortValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        std::pair<rsc_e, std::string> validateRS232(const JsonArray array, cin_vector* outVector);
        std::pair<rsc_e, std::string> validateRS485(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
    private:
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, prt_e> convertToPortIndex(const uint8_t portIndex);
        std::pair<rsc_e, bdr_e> convertToBaudRate(const uint32_t baudRate);
        std::pair<rsc_e, dbit_e> convertToDataBit(const uint8_t dataBit);
        std::pair<rsc_e, pbit_e> convertToParityBit(const uint8_t parityBit);
        std::pair<rsc_e, sbit_e> convertToStopBit(const uint8_t stopBit);
    };
}}