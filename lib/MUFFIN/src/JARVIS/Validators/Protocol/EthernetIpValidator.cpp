/**
 * @file EthernetIpValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */





#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "EthernetIpValidator.h"
#include "JARVIS/Config/Protocol/EthernetIP.h"



namespace muffin { namespace jvs {

    EthernetIpValidator::EthernetIpValidator()
    {
    }
    
    EthernetIpValidator::~EthernetIpValidator()
    {
    }

    std::pair<rsc_e, std::string> EthernetIpValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");

        switch (key)
        {
        case cfg_key_e::ETHERNET_IP:
            return validateEthernetIP(arrayCIN, outVector);
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR MODBUS INTERFACE");
        };
    }

    std::pair<rsc_e, std::string> EthernetIpValidator::validateEthernetIP(const JsonArray array, cin_vector* outVector)
    {
    #if !defined(MT11)
        const std::string message = "EthernetIP IS ONLY SUPPORTED ON MT11";
        return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
    #else
        for (JsonObject cin : array)
        {
            rsc_e rsc = validateMandatoryKeysEthernetIP(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID EthernetIP: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValuesEthernetIP(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID EthernetIP: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const uint16_t prt      = cin["prt"].as<uint16_t>();
            const std::string ip    = cin["ip"].as<std::string>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();
            const uint8_t EthernetInterfaces = cin["eths"].as<uint8_t>();
    
            const auto retIP   = convertToIPv4(ip);
            auto retNodes      = convertToNodes(nodes);
            const auto retEths = convertToEthernetInterfaces(EthernetInterfaces);
            if (retEths.first != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID ETHERNET INTERFACES");
            }

        /**
         * @todo eths 를 validation만 하고있음 setting하고 처리하는 것이 필요함 @김주성
         * 
         */
            if (prt == 0)
            {
                const std::string message = "INVALID EthernetIP PLC PORT NUMBER: " + std::to_string(prt);
                return std::make_pair(rsc, message);
            }

            if (retIP.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID EthernetIP PLC IP: " + ip;
                return std::make_pair(rsc, message);
            }    

            if (retNodes.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID EthernetIP NODES";
                return std::make_pair(rsc, message);
            }

            uint16_t scanRate = 0;

            if (cin.containsKey("sr"))
            {
                const auto retSR = convertToScanRate(cin["sr"].as<JsonVariant>());
                if (retSR.first != rsc_e::GOOD && retSR.first != rsc_e::GOOD_NO_DATA)
                {
                    const std::string message = "INVALID EthernetIP SCAN RATE";
                    return std::make_pair(retSR.first, message);
                } 
                
                scanRate = retSR.second;
            }

            config::EthernetIP* EthernetIP = new(std::nothrow) config::EthernetIP();
            if (EthernetIP == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR EthernetIP CONFIG");
            }

            EthernetIP->SetIPv4(retIP.second);
            EthernetIP->SetPort(prt);
            EthernetIP->SetNodes(std::move(retNodes.second));
            EthernetIP->SetEthernetInterface(std::move(retEths.second));
            EthernetIP->SetScanRate(scanRate);

            rsc = emplaceCIN(static_cast<config::Base*>(EthernetIP), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (EthernetIP != nullptr)
                {
                    delete EthernetIP;
                    EthernetIP = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: EthernetIP CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD"); 
    #endif
    }

    rsc_e EthernetIpValidator::validateMandatoryKeysEthernetIP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("nodes");
        isValid &= json.containsKey("eths");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e EthernetIpValidator::validateMandatoryValuesEthernetIP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["ip"].isNull()  == false;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["nodes"].isNull() == false;
        isValid &= json["eths"].isNull() == false;
        isValid &= json["ip"].is<std::string>();
        isValid &= json["prt"].is<uint16_t>();
        isValid &= json["nodes"].is<JsonArray>();
        isValid &= json["eths"].is<uint8_t>();

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e EthernetIpValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: EIP PROTOCOL, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: EIP PROTOCOL, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    std::pair<rsc_e, std::vector<std::string>> EthernetIpValidator::convertToNodes(const JsonArray nodes)
    {
        if (nodes.size() == 0)
        {
            LOG_ERROR(logger, "NODES REFERENCE LENGTH CANNOT BE 0");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::vector<std::string>());
        }
        
        std::vector<std::string> vectorNode;
        try
        {
            vectorNode.reserve(nodes.size());
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR NODES REFERENCES: %s", e.what());

            vectorNode.clear();
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorNode);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO RESERVE VECTOR FOR NODES REFERENCE: %s", e.what());

            vectorNode.clear();
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorNode);
        }
        
        for (JsonVariant node : nodes)
        {
            const std::string nodeID = node.as<std::string>();

            if (nodeID.length() != 4)
            {
                LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: %s", nodeID.c_str());

                vectorNode.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::move(vectorNode));
            }

            try
            {
                vectorNode.emplace_back(nodeID);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE NODE REFERENCE: %s", e.what());

                vectorNode.clear();
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorNode);
            }
        }    
        
        return std::make_pair(rsc_e::GOOD, std::move(vectorNode));
    }

    std::pair<rsc_e, uint16_t> EthernetIpValidator::convertToScanRate(JsonVariant scanRate)
    {
        if (scanRate.isNull() == true || scanRate.is<uint16_t>() == false)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, 0);
        }
        else
        {
            const uint16_t _scanRate = scanRate.as<uint16_t>();
            if ( _scanRate > 3000)
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, 0);
            }
            
            return std::make_pair(rsc_e::GOOD, _scanRate);
        }
    }

    
    std::pair<rsc_e, IPAddress> EthernetIpValidator::convertToIPv4(const std::string ip)
    {
        IPAddress IPv4;

        if (IPv4.fromString(ip.c_str()))  // fromString 함수가 IP 변환에 성공했는지 확인
        {
            return std::make_pair(rsc_e::GOOD, IPv4);
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, IPAddress());
        }
    
    }

    std::pair<rsc_e, if_e> EthernetIpValidator::convertToEthernetInterfaces(uint8_t eths)
    {
        switch (eths)
        {
        case 0:
            return std::make_pair(rsc_e::GOOD, if_e::EMBEDDED);
        case 1:
            return std::make_pair(rsc_e::GOOD, if_e::LINK_01);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, if_e::EMBEDDED);
        }
    }
}}
