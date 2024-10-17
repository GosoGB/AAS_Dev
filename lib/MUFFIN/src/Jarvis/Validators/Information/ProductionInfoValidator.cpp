/**
 * @file ProductionInfoValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산실적 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-11
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Information/Production.h"
#include "ProductionInfoValidator.h"



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

    std::pair<rsc_e, std::string> ProductionInfoValidator::Inspect(const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

        /**
         * @todo 현재는 한 번에 하나의 설정만 받는 것을 염두에 둔 설계입니다.
         *       다만, 향후에 multi-drop 방식과 같이 두 개 이상의 슬레이브와
         *       연동하는 경우에는 두 개 이상의 설정이 필요할 수 있습니다.
         * 
         *       그러한 상황이 된다면 하나의 설정만 받는 현행 방식의 코드를
         *       두 개 이상의 설정을 받을 수 있도록 수정해야 합니다.
         */
        JsonObject json = arrayCIN[0].as<JsonObject>();

        rsc_e rsc = validateMandatoryKeys(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID PRODUCTION INFO: MANDATORY KEY CANNOT BE MISSING");
        }

        std::pair<rsc_e, std::string> result = validateMandatoryValues(json);
        if (result.first != rsc_e::GOOD)
        {
            return result;
        }

        config::Production* prod = new(std::nothrow) config::Production();
        if (prod == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR PRODUCTION INFO CONFIG");
        }

        if (mIsTotalNull == false)
        {
            prod->SetNodeIdTotal(mTotalNodeID);
        }

        if (mIsGoodNull == false)
        {
            prod->SetNodeIdGood(mGoodNodeID);
        }

        if (mIsDefectNull == false)
        {
            prod->SetNodeIdNG(mDefectNodeID);
        }
        /*생산실적 정보가 존재하는 모든 Node ID가 설정되었습니다.*/

        rsc = emplaceCIN(static_cast<config::Base*>(prod), outVector);
        if (rsc != rsc_e::GOOD)
        {
            if (prod != nullptr)
            {
                delete prod;
                prod = nullptr;
            }
            return std::make_pair(rsc, "FAILED TO EMPLACE: PRODUCTION INFO CONFIG INSTANCE");
        }

        if (arrayCIN.size() > 1)
        {
            const std::string message = "ONLY ONE PRODUCTION INFO INFO CONFIG WILL BE APPLIED";
            return std::make_pair(rsc_e::UNCERTAIN, message);
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");  
        }
    }

    rsc_e ProductionInfoValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("tot");
        isValid &= json.containsKey("ok");
        isValid &= json.containsKey("ng");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    std::pair<rsc_e, std::string> ProductionInfoValidator::validateMandatoryValues(const JsonObject json)
    {
        mIsTotalNull  = json["tot"].isNull();
        mIsGoodNull   = json["ok"].isNull();
        mIsDefectNull = json["ng"].isNull();
        
        if(mIsTotalNull && mIsGoodNull && mIsDefectNull)
        {
            const std::string message = "INVALID PRODUCTION INFO: AT LEAST ONE KEY MUST NOT BE A NULL VALUE";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        /*적어도 하나의 생산실적 정보에 대한 설정이 존재합니다.*/
        mTotalNodeID   = json["tot"].as<std::string>();
        mGoodNodeID    = json["ok"].as<std::string>();
        mDefectNodeID  = json["ng"].as<std::string>();
        if ((mIsTotalNull  == false && mTotalNodeID.length()  != 4) ||
            (mIsGoodNull   == false && mGoodNodeID.length()   != 4) ||
            (mIsDefectNull == false && mDefectNodeID.length() != 4))
        {
            const std::string message = "INVALID PRODUCTION INFO: NODE ID LENGTH MUST BE EQUAL TO 4";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");   
    }

    rsc_e ProductionInfoValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "INPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }
}}