/**
 * @file OperationTimeValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동시간 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-11
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

    class OperationTimeValidator
    {
    public:
        OperationTimeValidator();
        virtual ~OperationTimeValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const JsonArray arrayCIN, cin_vector* outVector);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, op_time_type_e> convertToOperationTimeType(const uint8_t type);
        std::pair<rsc_e, cmp_op_e> convertToLogicalOperator(const std::string& stringOperator);
    };
}}