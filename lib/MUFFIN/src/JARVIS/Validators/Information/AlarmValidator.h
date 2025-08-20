/**
 * @file AlarmValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 이벤트를 생성하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-02-26
 * @version 1.2.13
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Config/Information/Alarm.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class AlarmValidator
    {
    public:
        AlarmValidator() {}
        virtual ~AlarmValidator() {}
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const JsonArray arrayCIN, cin_vector* outVector);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
        std::pair<rsc_e, std::string> processLowerControlLimit(const JsonObject json, config::Alarm* cin);
        std::pair<rsc_e, std::string> processUpperControlLimit(const JsonObject json, config::Alarm* cin);
        std::pair<rsc_e, std::string> processControlLimits(const JsonObject json, config::Alarm* cin);
        std::pair<rsc_e, std::string> processConditions(const JsonObject json, config::Alarm* cin);
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, alarm_type_e> convertToAlarmType(const uint8_t type);
    };
}}