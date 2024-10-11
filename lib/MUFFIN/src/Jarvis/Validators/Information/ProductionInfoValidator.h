/**
 * @file ProductionInfoValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산실적 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-10
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

    class ProductionInfoValidator
    {
    public:
        ProductionInfoValidator();
        virtual ~ProductionInfoValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateMandatoryKeys(const JsonObject json);
        Status validateMandatoryValues(const JsonObject json);
        Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        bool mIsTotalNull   = false;
        bool mIsGoodNull    = false;
        bool mIsDefectNull  = false;
        std::string mTotalNodeID;
        std::string mGoodNodeID;
        std::string mDefectNodeID;
    };
}}