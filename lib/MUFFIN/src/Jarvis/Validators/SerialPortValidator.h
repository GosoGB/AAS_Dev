/**
 * @file SerialPortValidator.h
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

    class SerialPortValidator
    {
    public:
        SerialPortValidator();
        virtual ~SerialPortValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateRS232(const JsonArray array, cin_vector* outVector);
        Status validateRS485(const JsonArray array, cin_vector* outVector);
        Status validateMandatoryKeys(const JsonObject json);
        Status validateMandatoryValues(const JsonObject json);
    private:
        Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<Status, prt_e> convertToPortIndex(const uint8_t portIndex);
        std::pair<Status, bdr_e> convertToBaudRate(const uint32_t baudRate);
        std::pair<Status, dbit_e> convertToDataBit(const uint8_t dataBit);
        std::pair<Status, pbit_e> convertToParityBit(const uint8_t parityBit);
        std::pair<Status, sbit_e> convertToStopBit(const uint8_t stopBit);
    };
}}