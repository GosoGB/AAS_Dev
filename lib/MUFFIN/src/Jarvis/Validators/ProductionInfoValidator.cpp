/**
 * @file ProductionInfoValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산 정보 수집을 위한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-10
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ProductionInfoValidator.h"
#include "Jarvis/Config/Information/Production.h"



namespace muffin { namespace jarvis {

    ProductionInfoValidator::ProductionInfoValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ProductionInfoValidator::~ProductionInfoValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status ProductionInfoValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        JsonObject json = arrayCIN[0].as<JsonObject>();
        const bool isTotalNull  = json["tot"].isNull();
        const bool isGoodNull   = json["ok"].isNull();
        const bool isDefectNull = json["ng"].isNull();
        if (isTotalNull && isGoodNull && isDefectNull)
        {
            LOG_ERROR(logger, "AT LEAST ONE KEY MUST NOT BE A NULL VALUE");
            return Status(Status::Code::BAD_NO_DATA_AVAILABLE);
        }
        /*적어도 하나의 Node ID가 존재합니다.*/

        std::string totalNodeID   = json["tot"].as<std::string>();
        std::string goodNodeID    = json["ok"].as<std::string>();
        std::string defectNodeID  = json["ng"].as<std::string>();
        if (isTotalNull  == false && totalNodeID.length()  != 4 ||
            isGoodNull   == false && goodNodeID.length()   != 4 ||
            isDefectNull == false && defectNodeID.length() != 4)
        {
            LOG_ERROR(logger, "NODE ID LENGTH MUST BE EQUAL TO 4");
            return Status(Status::Code::BAD_NODE_ID_INVALID);
        }
        /*모든 Node ID의 형식 자체는 유효합니다.*/

        config::Production* prod = new config::Production(key);
        if (isTotalNull == false)
        {
            prod->SetNodeIdTotal(totalNodeID);
        }

        if (isGoodNull == false)
        {
            prod->SetNodeIdGood(goodNodeID);
        }

        if (isDefectNull == false)
        {
            prod->SetNodeIdNG(defectNodeID);
        }
        /*생산실적 정보가 존재하는 모든 Node ID가 설정되었습니다.*/

        try
        {
            outVector->emplace_back(static_cast<config::Base*>(prod));
            if (arrayCIN.size() > 1)
            {
                LOG_WARNING(logger, "ONLY ONE PRODUCTION INFO CONFIG WILL BE APPLIED");
                return Status(Status::Code::UNCERTAIN);
            }
            else
            {
                return Status(Status::Code::GOOD);
            }
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: PRODUCTION", e.what());

            delete prod;
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: PRODUCTION", e.what());
            
            delete prod;
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }
}}