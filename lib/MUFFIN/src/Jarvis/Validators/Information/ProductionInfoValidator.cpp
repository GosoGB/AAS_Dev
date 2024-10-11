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

    Status ProductionInfoValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((key == cfg_key_e::PRODUCTION_INFO), "CONFIG CATEGORY DOES NOT MATCH");

        /**
         * @todo 현재는 한 번에 하나의 설정만 받는 것을 염두에 둔 설계입니다.
         *       다만, 향후에 multi-drop 방식과 같이 두 개 이상의 슬레이브와
         *       연동하는 경우에는 두 개 이상의 설정이 필요할 수 있습니다.
         * 
         *       그러한 상황이 된다면 하나의 설정만 받는 현행 방식의 코드를
         *       두 개 이상의 설정을 받을 수 있도록 수정해야 합니다.
         */
        JsonObject json = arrayCIN[0].as<JsonObject>();

        Status ret = validateMandatoryKeys(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "MANDATORY KEYS CANNOT BE MISSING");
            return ret;
        }

        ret = validateMandatoryValues(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE IS NULL OR INVALID NODE ID");
            return ret;
        }

        config::Production* prod = new(std::nothrow) config::Production(key);
        if (prod == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: PRODUCTION");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
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

        ret = emplaceCIN(static_cast<config::Base*>(prod), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE PRODUCTION INFO CIN: %s", ret.c_str());
            delete prod;
            return ret;
        }

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

    Status ProductionInfoValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("tot");
        isValid &= json.containsKey("ok");
        isValid &= json.containsKey("ng");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status ProductionInfoValidator::validateMandatoryValues(const JsonObject json)
    {
        mIsTotalNull  = json["tot"].isNull();
        mIsGoodNull   = json["ok"].isNull();
        mIsDefectNull = json["ng"].isNull();
        if (mIsTotalNull && mIsGoodNull && mIsDefectNull)
        {
            LOG_ERROR(logger, "AT LEAST ONE KEY MUST NOT BE A NULL VALUE");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        /*적어도 하나의 생산실적 정보에 대한 설정이 존재합니다.*/

        mTotalNodeID   = json["tot"].as<std::string>();
        mGoodNodeID    = json["ok"].as<std::string>();
        mDefectNodeID  = json["ng"].as<std::string>();
        if (mIsTotalNull  == false && mTotalNodeID.length()  != 4 ||
            mIsGoodNull   == false && mGoodNodeID.length()   != 4 ||
            mIsDefectNull == false && mDefectNodeID.length() != 4)
        {
            LOG_ERROR(logger, "NODE ID LENGTH MUST BE EQUAL TO 4");
            return Status(Status::Code::BAD_NODE_ID_INVALID);
        }

        return Status(Status::Code::GOOD);
    }

    Status ProductionInfoValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "INPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }
}}