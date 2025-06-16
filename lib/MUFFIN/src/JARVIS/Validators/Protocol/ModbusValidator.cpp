/**
 * @file ModbusValidator.cpp
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
#include "ModbusValidator.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "JARVIS/Config/Protocol/ModbusTCP.h"



namespace muffin { namespace jvs {

    ModbusValidator::ModbusValidator()
    {
    }
    
    ModbusValidator::~ModbusValidator()
    {
    }

    std::pair<rsc_e, std::string> ModbusValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, prtcl_ver_e protocolVersion, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "INPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        mProtocolVersion = protocolVersion;
        switch (key)
        {
        case cfg_key_e::MODBUS_RTU:
            return validateModbusRTU(arrayCIN, outVector);
        case cfg_key_e::MODBUS_TCP:
            return validateModbusTCP(arrayCIN, outVector);
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR MODBUS INTERFACE");
        };
    }

    std::pair<rsc_e, std::string> ModbusValidator::validateModbusTCP(const JsonArray array, cin_vector* outVector)
    {
    #if defined(MODLINK_L) || defined(ML10)
        const std::string message = "MODBUS TCP IS NOT SUPPORTED ON MODLINK-L OR ML10";
        return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
    #else
        for (JsonObject cin : array)
        {
            rsc_e rsc = validateMandatoryKeysModbusTCP(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MODBUS TCP: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValuesModbusTCP(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MODBUS TCP: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const uint16_t prt      = cin["prt"].as<uint16_t>();
            const uint8_t sid       = cin["sid"].as<uint8_t>();
            const std::string ip    = cin["ip"].as<std::string>();
            const std::string iface = cin["iface"].as<std::string>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();
          

            const auto retIP      = convertToIPv4(ip);
            const auto retIface   = convertToIface(iface);
            auto retNodes         = convertToNodes(nodes);
            const auto retSID     = convertToSlaveID(sid);

            

            if (prt == 0)
            {
                const std::string message = "INVALID MODBUS TCP SERVER PORT NUMBER: " + std::to_string(prt);
                return std::make_pair(rsc, message);
            }
            
            if (retSID.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS TCP SLAVE ID: " + std::to_string(sid);
                return std::make_pair(rsc, message);
            }

            if (retIP.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS TCP SERVER IP: " + ip;
                return std::make_pair(rsc, message);
            }    

            if (retIface.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS TCP IFACE: " + iface;
                return std::make_pair(rsc, message);
            }   

            if (retNodes.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS TCP NODES";
                return std::make_pair(rsc, message);
            }   

            config::ModbusTCP* modbusTCP = new(std::nothrow) config::ModbusTCP();
            if (modbusTCP == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR MODBUS TCP CONFIG");
            }

            if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
            {
                const uint8_t EthernetInterfaces = cin["eths"].as<uint8_t>();
                const auto retEths = convertToEthernetInterfaces(EthernetInterfaces);
                if (retEths.first != rsc_e::GOOD)
                {
                    return std::make_pair(rsc, "INVALID ETHERNET INTERFACES");
                }
                modbusTCP->SetEthernetInterface(retEths.second);
            }
            else
            {
                modbusTCP->SetEthernetInterface(if_e::EMBEDDED);
            }
            
            modbusTCP->SetSlaveID(retSID.second);
            modbusTCP->SetIPv4(retIP.second);
            modbusTCP->SetPort(prt);
            modbusTCP->SetNIC(retIface.second);
            modbusTCP->SetNodes(std::move(retNodes.second));
            
            
            rsc = emplaceCIN(static_cast<config::Base*>(modbusTCP), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (modbusTCP != nullptr)
                {
                    delete modbusTCP;
                    modbusTCP = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: MODBUS TCP CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD"); 
    #endif
    }

    std::pair<rsc_e, std::string> ModbusValidator::validateModbusRTU(const JsonArray array, cin_vector* outVector)
    {
        for (JsonObject cin : array)
        {
            rsc_e rsc = validateMandatoryKeysModbusRTU(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MODBUS RTU: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValuesModbusRTU(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID MODBUS RTU: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const uint8_t prt       = cin["prt"].as<uint8_t>();
            const uint8_t sid       = cin["sid"].as<uint8_t>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();
            
            const auto retPRT  = convertToPortIndex(prt);
            const auto retSID  = convertToSlaveID(sid);
            auto retNodes      = convertToNodes(nodes);

            if (retPRT.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS RTU SERIAL PORT INDEX: " + std::to_string(prt);
                return std::make_pair(rsc, message);
            }
            
            if (retSID.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS RTU SLAVE ID: " + std::to_string(sid);
                return std::make_pair(rsc, message);
            }

            if (retNodes.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID MODBUS TCP NODES";
                return std::make_pair(rsc, message);
            }
            
            config::ModbusRTU* modbusRTU = new(std::nothrow) config::ModbusRTU();
            if (modbusRTU == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR MODBUS RTU CONFIG");
            }

            modbusRTU->SetPort(retPRT.second);
            modbusRTU->SetSlaveID(retSID.second);
            modbusRTU->SetNodes(std::move(retNodes.second));

            rsc = emplaceCIN(static_cast<config::Base*>(modbusRTU), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (modbusRTU != nullptr)
                {
                    delete modbusRTU;
                    modbusRTU = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: MODBUS RTU CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");    
    }

    rsc_e ModbusValidator::validateMandatoryKeysModbusRTU(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("sid");
        isValid &= json.containsKey("nodes");
       
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e ModbusValidator::validateMandatoryValuesModbusRTU(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["sid"].isNull()  == false;
        isValid &= json["nodes"].isNull() == false;
        isValid &= json["prt"].is<uint8_t>();
        isValid &= json["sid"].is<uint8_t>();
        isValid &= json["nodes"].is<JsonArray>();

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e ModbusValidator::validateMandatoryKeysModbusTCP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("sid");
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("iface");
        isValid &= json.containsKey("nodes");

        if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
        {
            isValid &= json.containsKey("eths");  
        }

       
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e ModbusValidator::validateMandatoryValuesModbusTCP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["ip"].isNull()  == false;
        isValid &= json["sid"].isNull()  == false;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["iface"].isNull() == false;
        isValid &= json["nodes"].isNull() == false;
        isValid &= json["ip"].is<std::string>();
        isValid &= json["prt"].is<uint16_t>();
        isValid &= json["sid"].is<uint8_t>();
        isValid &= json["iface"].is<std::string>();
        isValid &= json["nodes"].is<JsonArray>();

        if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
        {
            isValid &= json["eths"].isNull() == false;
            isValid &= json["eths"].is<uint8_t>(); 
        }

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e ModbusValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
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

    std::pair<rsc_e, prt_e> ModbusValidator::convertToPortIndex(const uint8_t portIndex)
    {
        switch (portIndex)
        {
        case 2:
            return std::make_pair(rsc_e::GOOD, prt_e::PORT_2);
        
        #if !defined(MODLINK_L) && !defined(ML10)
        case 3:
            return std::make_pair(rsc_e::GOOD, prt_e::PORT_3);
        #endif

        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, prt_e::PORT_2);
        }
    }

    std::pair<rsc_e, std::vector<std::string>> ModbusValidator::convertToNodes(const JsonArray nodes)
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

    std::pair<rsc_e, nic_e> ModbusValidator::convertToIface(const std::string iface)
    {
        if (iface == "wifi")
        {
            return std::make_pair(rsc_e::GOOD, nic_e::WIFI4);
        }
        else if (iface == "eth")
        {
            return std::make_pair(rsc_e::GOOD, nic_e::ETHERNET);
        }
        else if (iface == "lte")
        {
            return std::make_pair(rsc_e::GOOD, nic_e::LTE_CatM1);
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, nic_e::ETHERNET);
        }        
    }

    std::pair<rsc_e, IPAddress> ModbusValidator::convertToIPv4(const std::string ip)
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

    std::pair<rsc_e, uint8_t> ModbusValidator::convertToSlaveID(const uint8_t slaveID)
    {
        if ( slaveID == 0)
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, 0);
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, slaveID);
        }
    }

    std::pair<rsc_e, if_e> ModbusValidator::convertToEthernetInterfaces(uint8_t eths)
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
