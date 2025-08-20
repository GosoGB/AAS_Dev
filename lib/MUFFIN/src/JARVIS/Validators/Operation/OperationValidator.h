/**
 * @file OperationValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 동작과 관련된 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-01-23
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <map>

#include "Common/PSRAM.hpp"
#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class OperationValidator
    {
    public:
        OperationValidator() {}
        virtual ~OperationValidator() {}
    public:
        std::pair<rsc_e, std::string> Inspect(const JsonArray arrayCIN);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
    private:
        std::pair<rsc_e, snic_e> convertToServerNIC(const char* snic);
    #if defined(MT11)
        std::pair<rsc_e, psram::map<uint16_t, psram::vector<std::string>>> convertToPublishIntervalCustom(const JsonObject json);
    #else
        std::pair<rsc_e, std::map<uint16_t, std::vector<std::string>>> convertToPublishIntervalCustom(const JsonObject json);
    #endif
    };
}}
