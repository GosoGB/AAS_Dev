/**
 * @file ProductionInfoValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산실적 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-10
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

    class ProductionInfoValidator
    {
    public:
        ProductionInfoValidator();
        virtual ~ProductionInfoValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const JsonArray arrayCIN, cin_vector* outVector);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        std::pair<rsc_e, std::string> validateMandatoryValues(const JsonObject json);
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        bool mIsTotalNull   = false;
        bool mIsGoodNull    = false;
        bool mIsDefectNull  = false;
        std::string mTotalNodeID;
        std::string mGoodNodeID;
        std::string mDefectNodeID;
    };
}}