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
    
    std::pair<rsc_e, std::string> OperationValidator::Inspect(const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");

        JsonObject json = arrayCIN[0].as<JsonObject>();
        rsc_e rsc = validateMandatoryKeys(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION : MANDATORY KEY CANNOT BE MISSING");
        }

        rsc = validateMandatoryValues(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION : MANDATORY KEY'S VALUE CANNOT BE NULL");
        }
        
        const bool isExpired    = json["exp"].as<bool>();
        const uint16_t pollingInverval = json["intvPoll"].as<uint16_t>();
        const uint16_t serverInverval  = json["intvSrv"].as<uint16_t>();
        const bool factoryReset = json["fmt"].as<bool>();

        const std::string snic = json["snic"].as<std::string>();
        const auto retSNIC = convertToServerNIC(snic);
        if (retSNIC.first != rsc_e::GOOD)
        {
            const std::string message = "INVALID SERVER NETWORK INTERFACE: " + snic;
            return std::make_pair(rsc, message);
        }
        
        config::Operation* operation = new(std::nothrow) config::Operation();
        if (operation == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR OPERATION CONFIG");
        }

        operation->SetPlanExpired(isExpired);
        operation->SetIntervalPolling(pollingInverval);
        operation->SetIntervalServer(serverInverval);
        operation->SetFactoryReset(factoryReset);
        operation->SetServerNIC(retSNIC.second);


        rsc = emplaceCIN(static_cast<config::Base*>(operation), outVector);
        if (rsc != rsc_e::GOOD)
        {
            if (operation != nullptr)
            {
                delete operation;
                operation = nullptr;
            }
            return std::make_pair(rsc, "FAILED TO EMPLACE: OPERATION CONFIG INSTANCE");
        }

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
        isValid &= json.containsKey("fmt");
        
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
        isValid &= json["fmt"].isNull()             == false;
        isValid &= json["exp"].is<bool>()           == true;
        isValid &= json["intvPoll"].is<uint16_t>()  == true;
        isValid &= json["intvSrv"].is<uint16_t>()   == true;
        isValid &= json["fmt"].is<bool>()           == true;
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e OperationValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
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

    std::pair<rsc_e, snic_e> OperationValidator::convertToServerNIC(const std::string& nic)
    {
        if (nic == "lte")
        {
            return std::make_pair(rsc_e::GOOD, snic_e::LTE_CatM1);
        }
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        else if (nic == "eth")
        {
            return std::make_pair(rsc_e::GOOD, snic_e::Ethernet);
        }
    #elif defined(MODLINK_B)
        else if (nic == "wifi")
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
