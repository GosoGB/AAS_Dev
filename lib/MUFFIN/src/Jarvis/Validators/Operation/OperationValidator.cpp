/**
 * @file OperationValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 동작과 관련된 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Operation/Operation.h"
#include "OperationValidator.h"



namespace muffin { namespace jarvis {

    OperationValidator::OperationValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    OperationValidator::~OperationValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    Status OperationValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

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
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return ret;
        }
        
        const bool isExpired    = json["exp"].as<bool>();
        const bool isOtaNeeded  = json["ota"].as<bool>();
        const uint16_t pollingInverval = json["intvPoll"].as<uint16_t>();
        const uint16_t serverInverval  = json["intvSrv"].as<uint16_t>();

        const std::string snic = json["snic"].as<std::string>();
        const auto retSNIC = convertToServerNIC(snic);
        if (retSNIC.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return retSNIC.first;
        }
        
        config::Operation* operation = new(std::nothrow) config::Operation(cfg_key_e::OPERATION);
        if (operation == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: OPERATION");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        operation->SetPlanExpired(isExpired);
        operation->SetCheckForOTA(isOtaNeeded);
        operation->SetIntervalPolling(pollingInverval);
        operation->SetIntervalServer(serverInverval);
        operation->SetServerNIC(retSNIC.second);

        ret = emplaceCIN(static_cast<config::Base*>(operation), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE OPERATION CIN: %s", ret.c_str());
            delete operation;
            return ret;
        }

        if (arrayCIN.size() > 1)
        {
            LOG_WARNING(logger, "ONLY ONE OPERATION CONFIG WILL BE APPLIED");
            return Status(Status::Code::UNCERTAIN);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    Status OperationValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("snic");
        isValid &= json.containsKey("exp");
        isValid &= json.containsKey("intvPoll");
        isValid &= json.containsKey("intvSrv");
        isValid &= json.containsKey("ota");
        
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status OperationValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["snic"].isNull()            == false;
        isValid &= json["exp"].isNull()             == false;
        isValid &= json["ota"].isNull()             == false;
        isValid &= json["intvPoll"].isNull()        == false;
        isValid &= json["intvSrv"].isNull()         == false;
        isValid &= json["exp"].is<bool>()           == true;
        isValid &= json["ota"].is<bool>()           == true;
        isValid &= json["intvPoll"].is<uint16_t>()  == true;
        isValid &= json["intvSrv"].is<uint16_t>()   == true;
        
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status OperationValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
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

    std::pair<Status, snic_e> OperationValidator::convertToServerNIC(const std::string& nic)
    {
        if (nic == "lte")
        {
            return std::make_pair(Status(Status::Code::GOOD), snic_e::LTE_CatM1);
        }
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        else if (nic == "eth")
        {
            return std::make_pair(Status(Status::Code::GOOD), snic_e::Ethernet);
        }
    #elif defined(MODLINK_B)
        else if (nic == "wifi")
        {
            return std::make_pair(Status(Status::Code::GOOD), snic_e::WiFi4);
        }
    #endif
        else
        {
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), snic_e::LTE_CatM1);
        }
    }
}}