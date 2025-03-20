/**
 * @file LteValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class LteValidator
    {
    public:
        LteValidator() {}
        virtual ~LteValidator() {}
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN);
    private:
        std::pair<rsc_e, std::string> validateLteCatM1(const JsonArray array);
        rsc_e validateMandatoryKeysLteCatM1(const JsonObject json);
        rsc_e validateMandatoryValuesLteCatM1(const JsonObject json);
    private:
        std::pair<rsc_e, md_e> convertToLteModel(const char* model);
        std::pair<rsc_e, ctry_e> convertToLteCountry(const char* country);
    };
}}