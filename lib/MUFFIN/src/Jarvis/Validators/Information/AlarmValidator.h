/**
 * @file AlarmValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 이벤트를 생성하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Config/Information/Alarm.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class AlarmValidator
    {
    public:
        AlarmValidator();
        virtual ~AlarmValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
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