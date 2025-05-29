/**
 * @file MelsecValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "MelsecValidator.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "JARVIS/Config/Protocol/Melsec.h"



namespace muffin { namespace jvs {

    MelsecValidator::MelsecValidator()
    {
    }
    
    MelsecValidator::~MelsecValidator()
    {
    }

    std::pair<rsc_e, std::string> MelsecValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");

        switch (key)
        {
        case cfg_key_e::MELSEC:
            return validateMelsec(arrayCIN, outVector);
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR MODBUS INTERFACE");
        };
    }

    std::pair<rsc_e, std::string> MelsecValidator::validateMelsec(const JsonArray array, cin_vector* outVector)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        const std::string message = "MELSEC IS NOT SUPPORTED ON MODLINK-L OR MODLINK-ML10";
        return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
    #else
        for (JsonObject cin : array)
        {
            rsc_e rsc = validateMandatoryKeysMelsec(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MELSEC: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValuesMelsec(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MELSEC: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const uint16_t prt      = cin["prt"].as<uint16_t>();
            const std::string ip    = cin["ip"].as<std::string>();
            const uint8_t plcSeries = cin["ps"].as<uint8_t>();
            const uint8_t dataFormat = cin["df"].as<uint8_t>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();
            const uint8_t EthernetInterfaces = cin["eths"].as<uint8_t>();
    
            const auto retIP    = convertToIPv4(ip);
            auto retNodes       = convertToNodes(nodes);
            auto retPlcSeries   = convertToPlcSeries(plcSeries);
            auto retDataFormat  = convertTodataFormat(dataFormat);
            
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
                const std::string message = "INVALID Melsec PLC PORT NUMBER: " + std::to_string(prt);
                return std::make_pair(rsc, message);
            }

            if (retIP.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID Melsec PLC IP: " + ip;
                return std::make_pair(rsc, message);
            }    

            if (retNodes.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID Melsec NODES";
                return std::make_pair(rsc, message);
            }

            if (retPlcSeries.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID Melsec PLC SERIES";
                return std::make_pair(rsc, message);
            }

            if (retDataFormat.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID Melsec DATA FORMAT";
                return std::make_pair(rsc, message);
            }
            
            config::Melsec* Melsec = new(std::nothrow) config::Melsec();
            if (Melsec == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR MELSEC CONFIG");
            }

            Melsec->SetIPv4(retIP.second);
            Melsec->SetPort(prt);
            Melsec->SetPlcSeries(std::move(retPlcSeries.second));
            Melsec->SetDataFormat(std::move(retDataFormat.second));
            Melsec->SetNodes(std::move(retNodes.second));
            Melsec->SetEthernetInterface(std::move(retEths.second));

            rsc = emplaceCIN(static_cast<config::Base*>(Melsec), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (Melsec != nullptr)
                {
                    delete Melsec;
                    Melsec = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: MELSEC CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD"); 
    #endif
    }

    rsc_e MelsecValidator::validateMandatoryKeysMelsec(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("ps");
        isValid &= json.containsKey("df");
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

    rsc_e MelsecValidator::validateMandatoryValuesMelsec(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["ip"].isNull()  == false;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["ps"].isNull()  == false;
        isValid &= json["df"].isNull()  == false;
        isValid &= json["nodes"].isNull() == false;
        isValid &= json["eths"].isNull() == false;
        isValid &= json["ip"].is<std::string>();
        isValid &= json["prt"].is<uint16_t>();
        isValid &= json["ps"].is<uint8_t>();
        isValid &= json["df"].is<uint8_t>();
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

    rsc_e MelsecValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: MODBUS PROTOCOL, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: MODBUS PROTOCOL, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    std::pair<rsc_e, ps_e> MelsecValidator::convertToPlcSeries(const uint8_t plcSeries)
    {
        switch (plcSeries)
        {
        case static_cast<uint8_t>(ps_e::QL_SERIES) :
            return std::make_pair(rsc_e::GOOD, ps_e::QL_SERIES);

        case static_cast<uint8_t>(ps_e::IQR_SERIES):
            return std::make_pair(rsc_e::GOOD, ps_e::IQR_SERIES);

        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, ps_e::QL_SERIES);
        }
    }

    std::pair<rsc_e, df_e> MelsecValidator::convertTodataFormat(const uint8_t dataFormat)
    {
        switch (dataFormat)
        {
        case static_cast<uint8_t>(df_e::BINARY) :
            return std::make_pair(rsc_e::GOOD, df_e::BINARY);

        case static_cast<uint8_t>(df_e::ASCII):
            return std::make_pair(rsc_e::GOOD, df_e::ASCII);

        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, df_e::BINARY);
        }
    }

    std::pair<rsc_e, std::vector<std::string>> MelsecValidator::convertToNodes(const JsonArray nodes)
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

    
    std::pair<rsc_e, IPAddress> MelsecValidator::convertToIPv4(const std::string ip)
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

    std::pair<rsc_e, if_e> MelsecValidator::convertToEthernetInterfaces(uint8_t eths)
    {
        switch (eths)
        {
        case 0:
            return std::make_pair(rsc_e::GOOD, if_e::EMBEDDED);
        case 1:
            return std::make_pair(rsc_e::GOOD, if_e::LINK_01);
        case 2:
            return std::make_pair(rsc_e::GOOD, if_e::LINK_02);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, if_e::EMBEDDED);
        }
    }
}}
