/**
 * @file OperationValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 동작과 관련된 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
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
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class OperationValidator
    {
    public:
        OperationValidator();
        virtual ~OperationValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateMandatoryKeys(const JsonObject json);
        Status validateMandatoryValues(const JsonObject json);
        Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<Status, snic_e> convertToServerNIC(const std::string& nic);
    };
}}