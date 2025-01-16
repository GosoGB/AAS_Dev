/**
 * @file LteValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */



#include <regex> 
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "LteValidator.h"
#include "JARVIS/Config/Network/CatM1.h"



namespace muffin { namespace jarvis {

    LteValidator::LteValidator()
    {
    }
    
    LteValidator::~LteValidator()
    {
    }

    std::pair<rsc_e, std::string> LteValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");

        switch (key)
        {
        case cfg_key_e::LTE_CatM1:
            return validateLteCatM1(arrayCIN, outVector);
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR LTE INTERFACE");
        };
    }

    std::pair<rsc_e, std::string> LteValidator::validateLteCatM1(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            ASSERT((array.size() == 1), "LTE CONFIG CANNOT BE GREATER THAN 1");
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, "INVALID WIFI CONFIG: ONLY ONE LTE MODULE CAN BE CONFIGURED");
        }

        JsonObject cin = array[0];
        rsc_e rsc = validateMandatoryKeysLteCatM1(cin);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID LTE Cat.M1: MANDATORY KEY CANNOT BE MISSING");
        }

        rsc = validateMandatoryValuesLteCatM1(cin);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID LTE Cat.M1: MANDATORY KEY'S VALUE CANNOT BE NULL");
        }

        const std::string md    = cin["md"].as<std::string>();
        const std::string ctry  = cin["ctry"].as<std::string>();

        const auto retMD     = convertToLteModel(md);
        const auto retCtry   = convertToLteCountry(ctry);

        if (retMD.first != rsc_e::GOOD)
        {
            const std::string message = "INVALID LTE CAT.M1 MODEL: " + md;
            return std::make_pair(retMD.first, message);
        }

        if (retCtry.first != rsc_e::GOOD)
        {
            const std::string message = "INVALID LTE CAT.M1 COUNTRY: " + ctry;
            return std::make_pair(retCtry.first, message);
        }

        config::CatM1* catM1 = new(std::nothrow) config::CatM1();
        if (catM1 == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR LTE CAT.M1 CONFIG");
        }
        catM1->SetModel(retMD.second);
        catM1->SetCounty(retCtry.second);

        rsc = emplaceCIN(static_cast<config::Base*>(catM1), outVector);
        if (rsc != rsc_e::GOOD)
        {
            if (catM1 != nullptr)
            {
                delete catM1;
                catM1 = nullptr;
            }
            return std::make_pair(rsc, "FAILED TO EMPLACE: LTE CAT.M1 CONFIG INSTANCE");
        }

        return std::make_pair(rsc_e::GOOD, "GOOD"); 
    }

    rsc_e LteValidator::validateMandatoryKeysLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("md");
        isValid &= json.containsKey("ctry");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e LteValidator::validateMandatoryValuesLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["md"].isNull()  == false;
        isValid &= json["ctry"].isNull()  == false;
        isValid &= json["md"].is<std::string>();
        isValid &= json["ctry"].is<std::string>();


        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e LteValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: LTE Cat.M1, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: LTE Cat.M1, CIN address: %p", e.what(), cin);
           return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    std::pair<rsc_e, md_e> LteValidator::convertToLteModel(const std::string model)
    {
        if (model == "LM5") 
        {
            return std::make_pair(rsc_e::GOOD, md_e::LM5);
        } 
        else if (model == "LCM300") 
        {
            return std::make_pair(rsc_e::GOOD, md_e::LCM300);
        } 
        else 
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, md_e::LM5);
        }
    }

    std::pair<rsc_e, ctry_e> LteValidator::convertToLteCountry(const std::string country)
    {
        if (country == "KR") 
        {
            return std::make_pair(rsc_e::GOOD, ctry_e::KOREA);
        } 
        else if (country == "USA") 
        {
            return std::make_pair(rsc_e::GOOD, ctry_e::USA);
        } 
        else 
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, ctry_e::KOREA);
        }
    }
}}