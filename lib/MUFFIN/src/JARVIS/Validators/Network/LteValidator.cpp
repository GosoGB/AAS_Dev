/**
 * @file LteValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JARVIS/Config/Network/CatM1.h"
#include "JARVIS/Validators/Network/LteValidator.h"



namespace muffin { namespace jvs {

    std::pair<rsc_e, std::string> LteValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

        switch (key)
        {
        case cfg_key_e::LTE_CatM1:
            return validateLteCatM1(arrayCIN);
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR LTE INTERFACE");
        };
    }

    std::pair<rsc_e, std::string> LteValidator::validateLteCatM1(const JsonArray array)
    {
        if (array.size() != 1)
        {
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, "INVALID LTE Cat.M1: ONLY ONE LTE MODULE CAN BE CONFIGURED");
        }

        JsonObject cin = array[0].as<JsonObject>();
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

        const char* model      = cin["md"].as<const char*>();
        const char* country    = cin["ctry"].as<const char*>();
        const auto retModel    = convertToLteModel(model);
        const auto retCountry  = convertToLteCountry(country);

        if (retModel.first != rsc_e::GOOD)
        {
            std::string message = "INVALID LTE CAT.M1 MODEL: ";
            message.append(model);
            return std::make_pair(retModel.first, message);
        }

        if (retCountry.first != rsc_e::GOOD)
        {
            std::string message = "INVALID LTE CAT.M1 COUNTRY: ";
            message.append(country);
            return std::make_pair(retCountry.first, message);
        }

        config::catM1 = new(std::nothrow) config::CatM1();
        if (config::catM1 == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR LTE CAT.M1 CONFIG");
        }

        config::catM1->SetModel(retModel.second);
        config::catM1->SetCounty(retCountry.second);
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
        isValid &= json["md"].is<const char*>();
        isValid &= json["ctry"].is<const char*>();

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    std::pair<rsc_e, md_e> LteValidator::convertToLteModel(const char* model)
    {
        if (strcmp(model, "LM5") == 0)
        {
            return std::make_pair(rsc_e::GOOD, md_e::LM5);
        } 
        else if (strcmp(model, "LCM300")  == 0)
        {
            return std::make_pair(rsc_e::GOOD, md_e::LCM300);
        } 
        else 
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, md_e::LM5);
        }
    }

    std::pair<rsc_e, ctry_e> LteValidator::convertToLteCountry(const char* country)
    {
        if (strcmp(country, "KR") == 0)
        {
            return std::make_pair(rsc_e::GOOD, ctry_e::KOREA);
        } 
        else if (strcmp(country, "USA") == 0)
        {
            return std::make_pair(rsc_e::GOOD, ctry_e::USA);
        } 
        else 
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, ctry_e::KOREA);
        }
    }
}}