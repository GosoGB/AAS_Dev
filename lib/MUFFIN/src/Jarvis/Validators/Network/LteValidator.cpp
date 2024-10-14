/**
 * @file LteValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */



#include <regex> 
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "LteValidator.h"
#include "Jarvis/Config/Network/CatM1.h"
#include "Jarvis/Include/Helper.h"



namespace muffin { namespace jarvis {

    LteValidator::LteValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    LteValidator::~LteValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status LteValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        Status ret(Status::Code::UNCERTAIN);

        switch (key)
        {
        case cfg_key_e::LTE_CatM1:
            ret = validateLteCatM1(arrayCIN, outVector);
            break;
        default:
            ASSERT(false, "UNDEFINED SERIAL PORT CONFIGURATION");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        };

        return ret;
    }

    Status LteValidator::validateLteCatM1(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            LOG_ERROR(logger, "INVALID LTE CONFIG: ONLY ONE LTE MODULE CAN BE CONFIGURED");
            ASSERT((array.size() == 1), "LTE CONFIG CANNOT BE GREATER THAN 1");
            return Status(Status::Code::BAD_NOT_SUPPORTED);
        }

        JsonObject cin = array[0];
        Status ret = validateMandatoryKeysLteCatM1(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1: MANDATORY KEY CANNOT BE MISSING");
            return ret;
        }

        ret = validateMandatoryValuesLteCatM1(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1: MANDATORY KEY'S VALUE CANNOT BE NULL");
            return ret;
        }

        const std::string md    = cin["md"].as<std::string>();
        const std::string ctry  = cin["ctry"].as<std::string>();

        const auto retMD     = convertToLteModel(md);
        const auto retCtry   = convertToLteCountry(ctry);

        if (retMD.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Model: %s", md.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        if (retCtry.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Country: %s", ctry.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        config::CatM1* catM1 = new(std::nothrow) config::CatM1();
        if (catM1 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: LTE cat.M1");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catM1->SetModel(retMD.second);
        catM1->SetCounty(retCtry.second);

        ret = emplaceCIN(static_cast<config::Base*>(catM1), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

        LOG_VERBOSE(logger, "Valid LTE Cat.M1 config instance")
        return Status(Status::Code::GOOD);
        
    }

    Status LteValidator::validateMandatoryKeysLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("md");
        isValid &= json.containsKey("ctry");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status LteValidator::validateMandatoryValuesLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["md"].isNull()  == false;
        isValid &= json["ctry"].isNull()  == false;
        isValid &= json["md"].is<std::string>();
        isValid &= json["ctry"].is<std::string>();


        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status LteValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: LTE Cat.M1, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: LTE Cat.M1, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::pair<Status, md_e> LteValidator::convertToLteModel(const std::string model)
    {
        if (model == "LM5") 
        {
            return std::make_pair(Status(Status::Code::GOOD), md_e::LM5);
        } 
        else if (model == "LCM300") 
        {
            return std::make_pair(Status(Status::Code::GOOD), md_e::LCM300);
        } 
        else 
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), md_e::LM5);
        }
    }

    std::pair<Status, ctry_e> LteValidator::convertToLteCountry(const std::string country)
    {
        if (country == "KR") 
        {
            return std::make_pair(Status(Status::Code::GOOD), ctry_e::KOREA);
        } 
        else if (country == "USA") 
        {
            return std::make_pair(Status(Status::Code::GOOD), ctry_e::USA);
        } 
        else 
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), ctry_e::KOREA);
        }
    }
}}