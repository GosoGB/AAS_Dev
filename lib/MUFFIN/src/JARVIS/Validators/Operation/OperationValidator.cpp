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




#include "Common/Assert.hpp"
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

        const auto retPublishIntervalCustom = convertToPublishIntervalCustom(json);
        if (retPublishIntervalCustom.first != rsc_e::GOOD && retPublishIntervalCustom.first != rsc_e::GOOD_NO_DATA)
        {
            char buffer[64] = {'\0'};
            snprintf(buffer, sizeof(buffer), "INVALID INTERVAL CUSTOM");
            return std::make_pair(rsc, buffer);
        }
        
        config::operation.SetPlanExpired(isExpired);
        config::operation.SetFactoryReset(hasFactoryReset);
        config::operation.SetServerNIC(retSNIC.second);
        config::operation.SetIntervalPolling(pollingInverval);
        config::operation.SetIntervalServer(publishInverval);
        config::operation.SetIntervalServerCustom(retPublishIntervalCustom.second);

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
    #if defined(MT10) || defined(MB10) || defined(MT11)
        else if (strcmp(snic, "eth") == 0)
        {
            return std::make_pair(rsc_e::GOOD, snic_e::Ethernet);
        }
    #elif defined(MB10)
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


    

#if defined(MT11)

    std::pair<rsc_e, psram::map<uint16_t, psram::vector<std::string>>> OperationValidator::convertToPublishIntervalCustom(const JsonObject json)
    {
        psram::map<uint16_t, psram::vector<std::string>> map;

        if (json.containsKey("intvSrvCust") == false)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }
        
        if (json["intvSrvCust"].isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }

        if (json["intvSrvCust"].is<JsonObject>() == false)
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
        }

        JsonObject intvSrvCust = json["intvSrvCust"].as<JsonObject>();
        if (intvSrvCust.size() == 0) 
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }

        for (JsonPair intervalPair : intvSrvCust)
        {
            const char* intv = intervalPair.key().c_str();
            if (intv == nullptr || *intv == '\0')
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            char* endp = nullptr;
            unsigned long ul = strtoul(intv, &endp, 10);

            if (endp == intv) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            if (*endp != '\0') 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            if (ul > UINT16_MAX || ul == 0) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            uint16_t intervalValue = static_cast<uint16_t>(ul);
            
            JsonVariant nodeIdVariant = intervalPair.value();
            if (!nodeIdVariant.is<JsonArray>()) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            JsonArray nodes = nodeIdVariant.as<JsonArray>();
            psram::vector<std::string> vectorNode;
            vectorNode.reserve(nodes.size());
            
            for (JsonVariant node : nodes) 
            {
                const std::string nodeID = node.as<std::string>();

                if (nodeID.length() != 4)
                {
                    LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: %s", nodeID.c_str());
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
                }

                try
                {
                    vectorNode.emplace_back(nodeID);
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE NODE REFERENCE: %s", e.what());

                    return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, map);
                }
            }
            
            auto it = map.find(intervalValue);
            if (it == map.end()) 
            {
                map.emplace(intervalValue, std::move(vectorNode));
            }
            else 
            {
                psram::vector<std::string>& dst = it->second;
                dst.clear();
                dst.reserve(vectorNode.size());
                dst.insert(dst.end(),
                        std::make_move_iterator(vectorNode.begin()),
                        std::make_move_iterator(vectorNode.end()));
            }

        }

        return std::make_pair(rsc_e::GOOD, map); 
    }

#else
    std::pair<rsc_e, std::map<uint16_t, std::vector<std::string>>> OperationValidator::convertToPublishIntervalCustom(const JsonObject json)
    {
        std::map<uint16_t, std::vector<std::string>> map;

        if (json.containsKey("intvSrvCust") == false)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }
        
        if (json["intvSrvCust"].isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }

        if (json["intvSrvCust"].is<JsonObject>() == false)
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
        }

        JsonObject intvSrvCust = json["intvSrvCust"].as<JsonObject>();
        if (intvSrvCust.size() == 0) 
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, map);
        }


        for (JsonPair intervalPair : intvSrvCust)
        {
            const char* intv = intervalPair.key().c_str();
            if (intv == nullptr || *intv == '\0')
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            char* endp = nullptr;
            unsigned long ul = strtoul(intv, &endp, 10);

            if (endp == intv) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            if (*endp != '\0') 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            if (ul > UINT16_MAX) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            uint16_t intervalValue = static_cast<uint16_t>(ul);
            
            JsonVariant nodeIdVariant = intervalPair.value();
            if (!nodeIdVariant.is<JsonArray>()) 
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
            }

            JsonArray nodes = nodeIdVariant.as<JsonArray>();
            std::vector<std::string> vectorNode;
            vectorNode.reserve(nodes.size());
            
            for (JsonVariant node : nodes) 
            {
                const std::string nodeID = node.as<std::string>();

                if (nodeID.length() != 4)
                {
                    LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: %s", nodeID.c_str());
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, map);
                }

                try
                {
                    vectorNode.emplace_back(nodeID);
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE NODE REFERENCE: %s", e.what());

                    return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, map);
                }
            }
            
            auto it = map.find(intervalValue);
            if (it == map.end()) 
            {
                map.emplace(intervalValue, std::move(vectorNode));
            }
            else 
            {
                std::vector<std::string>& dst = it->second;
                dst.clear();
                dst.reserve(vectorNode.size());
                dst.insert(dst.end(),
                        std::make_move_iterator(vectorNode.begin()),
                        std::make_move_iterator(vectorNode.end()));
            }

        }

        return std::make_pair(rsc_e::GOOD, map); 
    }
#endif

}}
