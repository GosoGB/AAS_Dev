/**
 * @file ModbusValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex>
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusValidator.h"
#include "Jarvis/Include/Helper.h"
#include "Jarvis/Config/Protocol/ModbusRTU.h"
#include "Jarvis/Config/Protocol/ModbusTCP.h"



namespace muffin { namespace jarvis {

    ModbusValidator::ModbusValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ModbusValidator::~ModbusValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status ModbusValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        Status ret(Status::Code::UNCERTAIN);

        switch (key)
        {
        case cfg_key_e::MODBUS_RTU:
            ret = validateModbusRTU(arrayCIN, outVector);
            break;
        case cfg_key_e::MODBUS_TCP:
            ret = validateModbusTCP(arrayCIN, outVector);
            break;
        default:
            ASSERT(false, "UNDEFINED SERIAL PORT CONFIGURATION");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        };

        return ret;
    }

    Status ModbusValidator::validateModbusTCP(const JsonArray array, cin_vector* outVector)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        LOG_ERROR(logger, "MODBUS TCP IS NOT SUPPORTED ON MODLINK-L OR MODLINK-ML10");
        return Status(Status::Code::BAD_NOT_SUPPORTED);
    #else
        for (JsonObject cin : array)
        {
            Status ret = validateMandatoryKeysModbusTCP(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS TCP: MANDATORY KEY CANNOT BE MISSING");
                return ret;
            }

            ret = validateMandatoryValuesModbusTCP(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS TCP: MANDATORY KEY'S VALUE CANNOT BE NULL");
                return ret;
            }

            const uint16_t prt      = cin["prt"].as<uint16_t>();
            const std::string ip    = cin["ip"].as<std::string>();
            const std::string iface = cin["iface"].as<std::string>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();
     
            const auto retIP      = convertToIPv4(ip);
            const auto retIface   = convertToIface(iface);
            auto retNodes   = convertToNodes(nodes);

            if (retIP.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS TCP IP : %s", ip.c_str());
                goto INVALID_MODBUS_TCP;
            }    

            if (retIface.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS TCP IFACE : %s", iface.c_str());
                goto INVALID_MODBUS_TCP;
            }   

            if (retNodes.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS TCP NODES");
                goto INVALID_MODBUS_TCP;
            }   

            config::ModbusTCP* modbusTCP = new config::ModbusTCP(cfg_key_e::MODBUS_TCP);
            modbusTCP->SetIPv4(retIP.second);
            modbusTCP->SetPort(prt);
            modbusTCP->SetNIC(retIface.second);
            modbusTCP->SetNodes(std::move(retNodes.second));

            
            Status ret = emplaceCIN(static_cast<config::Base*>(modbusTCP), outVector);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
                return ret;
            }
        }

        LOG_VERBOSE(logger, "Valid RS-232 config instance")
        return Status(Status::Code::GOOD);
    
    INVALID_MODBUS_TCP:
        return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
    #endif
    }

    Status ModbusValidator::validateModbusRTU(const JsonArray array, cin_vector* outVector)
    {
        for (JsonObject cin : array)
        {
            Status ret = validateMandatoryKeysModbusRTU(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID RS-485: MANDATORY KEY CANNOT BE MISSING");
                return ret;
            }

            ret = validateMandatoryValuesModbusRTU(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID RS-485: MANDATORY KEY'S VALUE CANNOT BE NULL");
                return ret;
            }

            const uint8_t prt       = cin["prt"].as<uint8_t>();
            const uint8_t sid       = cin["sid"].as<uint8_t>();
            const JsonArray nodes   = cin["nodes"].as<JsonArray>();

            const auto retPRT  = convertToPortIndex(prt);
            const auto retSID  = convertToSlaveID(sid);
            auto retNodes      = convertToNodes(nodes);


            if (retPRT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS RTU SERIAL PORT INDEX: %u", prt);
                goto INVALID_MODBUS_RTU;
            }
            
            if (retSID.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS RTU SLAVE ID: %u", sid);
                goto INVALID_MODBUS_RTU;
            }

            if (retNodes.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS RTU NODES");
                goto INVALID_MODBUS_RTU;
            }

            config::ModbusRTU* modbusRTU = new config::ModbusRTU(cfg_key_e::MODBUS_RTU);
            modbusRTU->SetPort(retPRT.second);
            modbusRTU->SetSlaveID(retSID.second);
            modbusRTU->SetNodes(std::move(retNodes.second));

            Status ret = emplaceCIN(static_cast<config::Base*>(modbusRTU), outVector);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
                return ret;
            }
        }

        LOG_VERBOSE(logger, "Valid Modbus RTU config instance")
        return Status(Status::Code::GOOD);
    
    INVALID_MODBUS_RTU:
        return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
    }

    Status ModbusValidator::validateMandatoryKeysModbusRTU(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("sid");
        isValid &= json.containsKey("nodes");
       
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status ModbusValidator::validateMandatoryValuesModbusRTU(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["sid"].isNull()  == false;
        isValid &= json["nodes"].isNull() == false;

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status ModbusValidator::validateMandatoryKeysModbusTCP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("iface");
        isValid &= json.containsKey("nodes");
       
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status ModbusValidator::validateMandatoryValuesModbusTCP(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["ip"].isNull()  == false;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["iface"].isNull() == false;
        isValid &= json["nodes"].isNull() == false;

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status ModbusValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: RS-232, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: RS-232, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::pair<Status, prt_e> ModbusValidator::convertToPortIndex(const uint8_t portIndex)
    {
        switch (portIndex)
        {
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_2);
        
        #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_3);
        #endif

        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), prt_e::PORT_2);
        }
    }

    std::pair<Status, std::vector<std::string>> ModbusValidator::convertToNodes(const JsonArray nodes)
    {
        std::vector<std::string> vNode;
        for (JsonVariant node : nodes)
        {
            std::string nodeID = node.as<std::string>();
            if (nodeID.size() != 4)
            {
                LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: {%s}", nodeID.c_str());
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), std::move(vNode));
            }
            vNode.push_back(nodeID);
        }    
        return std::make_pair(Status(Status::Code::GOOD), std::move(vNode));
    }

    std::pair<Status, nic_e> ModbusValidator::convertToIface(const std::string iface)
    {
        if(iface == "wifi")
        {
            return std::make_pair(Status(Status::Code::GOOD), nic_e::WIFI4);
        }
        else if(iface == "eth")
        {
            return std::make_pair(Status(Status::Code::GOOD), nic_e::ETHERNET);
        }
        else if(iface == "lte")
        {
            return std::make_pair(Status(Status::Code::GOOD), nic_e::LTE_CatM1);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), nic_e::ETHERNET);
        }        
    }

    std::pair<Status, IPAddress> ModbusValidator::convertToIPv4(const std::string ip)
    {
        IPAddress IPv4;

        std::regex validationRegex;
        
        validationRegex.assign("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        

        // validating IPv4 address using regular expression
        if (std::regex_match(ip, validationRegex))
        {
            if (IPv4.fromString(ip.c_str()))  // fromString 함수가 IP 변환에 성공했는지 확인
            {
                return std::make_pair(Status(Status::Code::GOOD), IPv4);
            }
            else
            {
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), IPAddress());
            }
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), IPAddress());
        }
    }

    std::pair<Status, uint8_t> ModbusValidator::convertToSlaveID(const uint8_t slaveID)
    {
        if(slaveID > 247 || slaveID == 0)
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), 0);
        }
        else
        {
            return std::make_pair(Status(Status::Code::GOOD), slaveID);
        }
    }

    std::pair<Status, prt_e> ModbusValidator::convertToPortIndex(const uint8_t portIndex)
    {
        switch (portIndex)
        {
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_2);
        
        #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_3);
        #endif

        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), prt_e::PORT_2);
        }
    }
}}
