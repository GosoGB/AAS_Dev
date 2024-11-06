/**
 * @file LteValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
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

    class LteValidator
    {
    public:
        LteValidator();
        virtual ~LteValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        std::pair<rsc_e, std::string> validateLteCatM1(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeysLteCatM1(const JsonObject json);
        rsc_e validateMandatoryValuesLteCatM1(const JsonObject json);
    private:
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, md_e> convertToLteModel(const std::string model);
        std::pair<rsc_e, ctry_e> convertToLteCountry(const std::string country);
     
    };
}}