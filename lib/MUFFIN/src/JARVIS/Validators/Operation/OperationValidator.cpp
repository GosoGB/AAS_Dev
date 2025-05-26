/**
 * @file OperationValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 동작과 관련된 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "JARVIS/Validators/Operation/OperationValidator.h"



namespace muffin { namespace jvs {

    std::pair<rsc_e, std::string> OperationValidator::Inspect(const JsonArray arrayCIN)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

        JsonObject json = arrayCIN[0].as<JsonObject>();
        rsc_e rsc = validateMandatoryKeys(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION: MANDATORY KEY CANNOT BE MISSING");
        }

        rsc = validateMandatoryValues(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION: MANDATORY KEY'S VALUE CANNOT BE NULL");
        }
        
        const bool isExpired            = json["exp"].as<bool>();
        const bool hasFactoryReset      = json["rst"].as<bool>();
        const char* serviceNetwork      = json["snic"].as<const char*>();
        const uint16_t pollingInverval  = json["intvPoll"].as<uint16_t>();
        const uint16_t publishInverval  = json["intvSrv"].as<uint16_t>();

        const auto retSNIC = convertToServerNIC(serviceNetwork);
        if (retSNIC.first != rsc_e::GOOD)
        {
            char buffer[64] = {'\0'};
            snprintf(buffer, sizeof(buffer), "INVALID SERVER NETWORK INTERFACE: %s", serviceNetwork);
            return std::make_pair(rsc, buffer);
        }

        config::operation.SetPlanExpired(isExpired);
        config::operation.SetFactoryReset(hasFactoryReset);
        config::operation.SetServerNIC(retSNIC.second);
        config::operation.SetIntervalPolling(pollingInverval);
        config::operation.SetIntervalServer(publishInverval);

        if (arrayCIN.size() > 1)
        {
            return std::make_pair(rsc_e::UNCERTAIN, "ONLY ONE OPERATION CONFIG WILL BE APPLIED");
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
    }

    rsc_e OperationValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("snic");
        isValid &= json.containsKey("exp");
        isValid &= json.containsKey("intvPoll");
        isValid &= json.containsKey("intvSrv");
        isValid &= json.containsKey("rst");
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e OperationValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        
        isValid &= json["snic"].isNull()            == false;
        isValid &= json["exp"].isNull()             == false;
        isValid &= json["intvPoll"].isNull()        == false;
        isValid &= json["intvSrv"].isNull()         == false;
        isValid &= json["rst"].isNull()             == false;

        isValid &= json["snic"].is<const char*>()   == true;
        isValid &= json["exp"].is<bool>()           == true;
        isValid &= json["intvPoll"].is<uint16_t>()  == true;
        isValid &= json["intvSrv"].is<uint16_t>()   == true;
        isValid &= json["rst"].is<bool>()           == true;
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    std::pair<rsc_e, snic_e> OperationValidator::convertToServerNIC(const char* snic)
    {
        if (strcmp(snic, "lte") == 0)
        {
            return std::make_pair(rsc_e::GOOD, snic_e::LTE_CatM1);
        }
    #if defined(MODLINK_T2) || defined(MODLINK_B) || defined(MT11)
        else if (strcmp(snic, "eth") == 0)
        {
            return std::make_pair(rsc_e::GOOD, snic_e::Ethernet);
        }
    #elif defined(MODLINK_B)
        else if (strcmp(snic, "wifi") == 0)
        {
            return std::make_pair(rsc_e::GOOD, snic_e::WiFi4);
        }
    #endif
        else
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, snic_e::LTE_CatM1);
        }
    }
}}
