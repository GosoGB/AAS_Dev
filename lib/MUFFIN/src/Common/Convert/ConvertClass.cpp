/**
 * @file Convert.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다양한 데이터 타입 간의 변환을 지원하는 유틸리티 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <cstdlib>
#include <cerrno>
#include <cfloat>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ConvertClass.h"
#include "Jarvis/Config/Information/Alarm.h"
#include "Jarvis/Config/Information/Node.h"
#include "Jarvis/Config/Information/OperationTime.h"
#include "Jarvis/Config/Information/Production.h"
#include "Jarvis/Config/Interfaces/Rs232.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Config/Network/CatM1.h"
#include "Jarvis/Config/Network/Ethernet.h"
#include "Jarvis/Config/Network/WiFi4.h"
#include "Jarvis/Config/Operation/Operation.h"
#include "Jarvis/Config/Protocol/ModbusRTU.h"
#include "Jarvis/Config/Protocol/ModbusTCP.h"



namespace muffin {

    ConvertClass Convert;
    
    int8_t ConvertClass::ToInt8(const std::string& input)
    {
        return static_cast<int8_t>(ToInt64(input));
    }

    int16_t ConvertClass::ToInt16(const std::string& input)
    {
        return static_cast<int16_t>(ToInt64(input));
    }

    int32_t ConvertClass::ToInt32(const std::string& input)
    {
        return static_cast<int32_t>(ToInt64(input));
    }

    int64_t ConvertClass::ToInt64(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const int64_t output = std::strtoll(input.c_str(), &endPointer, 10);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return INT64_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return INT64_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return INT64_MAX;
        }
        else
        {
            return output;
        }
    }

    uint8_t ConvertClass::ToUInt8(const std::string& input)
    {
        return static_cast<uint8_t>(ToUInt64(input));
    }

    uint8_t ConvertClass::ToUInt8(const jarvis::cfg_key_e input)
    {
        return static_cast<uint8_t>(input);
    }

    uint8_t ConvertClass::ToUInt8(const jarvis::prtcl_ver_e input)
    {
        return static_cast<uint8_t>(input);
    }

    uint16_t ConvertClass::ToUInt16(const std::string& input)
    {
        return static_cast<uint16_t>(ToUInt64(input));
    }
    
    uint16_t ConvertClass::ToUInt16(const jarvis::rsc_e input)
    {
        return static_cast<uint16_t>(input);
    }

    uint32_t ConvertClass::ToUInt32(const std::string& input)
    {
        return static_cast<uint32_t>(ToUInt64(input));
    }

    uint32_t ConvertClass::ToUInt32(const time_t input)
    {
        return static_cast<uint32_t>(input);
    }

    uint32_t ConvertClass::ToUInt32(const jarvis::bdr_e input)
    {
        return static_cast<uint32_t>(input);
    }

    uint64_t ConvertClass::ToUInt64(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const uint64_t output = std::strtoull(input.c_str(), &endPointer, 10);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return UINT64_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return UINT64_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return UINT64_MAX;
        }
        else
        {
            return output;
        }
    }

    float ConvertClass::ToFloat(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const float output = std::strtof(input.c_str(), &endPointer);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return FLT_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return FLT_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return FLT_MAX;
        }
        else
        {
            return output;
        }
    }

    double ConvertClass::ToDouble(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const double output = std::strtod(input.c_str(), &endPointer);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return DBL_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return DBL_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return DBL_MAX;
        }
        else
        {
            return output;
        }
    }

    std::string ConvertClass::ToString(const jarvis::cfg_key_e input)
    {
        using namespace jarvis;

        switch (input)
        {
        case cfg_key_e::RS232:
            return "rs232";
        case cfg_key_e::RS485:
            return "rs485";
        case cfg_key_e::WIFI4:
            return "wifi";
        case cfg_key_e::ETHERNET:
            return "eth";
        case cfg_key_e::LTE_CatM1:
            return "catm1";
        case cfg_key_e::MODBUS_RTU:
            return "mbrtu";
        case cfg_key_e::MODBUS_TCP:
            return "mbtcp";
        case cfg_key_e::OPERATION:
            return "op";
        case cfg_key_e::NODE:
            return "node";
        case cfg_key_e::ALARM:
            return "alarm";
        case cfg_key_e::OPERATION_TIME:
            return "optime";
        case cfg_key_e::PRODUCTION_INFO:
            return "prod";
        default:
            ASSERT(false, "UNDEFINED CONFIG INSTANCE KEY: %u", ToUInt8(input));
            return "";
        }
    }

    std::string ConvertClass::ToString(const jarvis::prtcl_ver_e input)
    {
        using namespace jarvis;

        switch (input)
        {
        case prtcl_ver_e::VERSEOIN_1:
            return "v1";
        case prtcl_ver_e::VERSEOIN_2:
            return "v2";
        case prtcl_ver_e::VERSEOIN_3:
            return "v3";
        default:
            ASSERT(
                (
                    (prtcl_ver_e::VERSEOIN_1 <= input) && (input <= prtcl_ver_e::VERSEOIN_3)
                ), "UNDEFINED OR UNSUPPORTED JARVIS PROTOCOL VERSION: %u", ToUInt8(input)
            );
            return "";
        }
    }

    std::pair<Status, jarvis::cfg_key_e> ConvertClass::ToJarvisKey(const jarvis::prtcl_ver_e version, const std::string& input)
    {
        using namespace jarvis;

        if (static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_1) <= static_cast<uint8_t>(version))
        {
            if (input == "rs232")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::RS232);
            }
            else if (input == "rs485")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::RS485);
            }
            else if (input == "wifi")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::WIFI4);
            }
            else if (input == "eth")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::ETHERNET);
            }
            else if (input == "catm1")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::LTE_CatM1);
            }
            else if (input == "mbrtu")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::MODBUS_RTU);
            }
            else if (input == "mbtcp")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::MODBUS_TCP);
            }
            else if (input == "op")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::OPERATION);
            }
            else if (input == "node")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::NODE);
            }
            else if (input == "alarm")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::ALARM);
            }
            else if (input == "optime")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::OPERATION_TIME);
            }
            else if (input == "prod")
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::PRODUCTION_INFO);
            }
            else
            {
                LOG_ERROR(logger, "INVALID JARVIS CONFIG INSTANCE KEY: %s", input.c_str());
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), cfg_key_e::ALARM);
            }
        }
        else
        {
            LOG_ERROR(logger, "JARVIS CONFIG IS IN UNSUPPORTED PROTOCOL VERSION: %s", ToString(version).c_str());
            return std::make_pair(Status(Status::Code::BAD_PROTOCOL_VERSION_UNSUPPORTED), cfg_key_e::ALARM);
        }
    }

    std::pair<Status, jarvis::prtcl_ver_e> ConvertClass::ToJarvisVersion(const std::string& input)
    {
        using namespace jarvis;

        if (input == "v1")
        {
            return std::make_pair(Status(Status::Code::GOOD), prtcl_ver_e::VERSEOIN_1);
        }
        else if (input == "v2")
        {
            return std::make_pair(Status(Status::Code::GOOD), prtcl_ver_e::VERSEOIN_2);
        }
        else if (input == "v3")
        {
            return std::make_pair(Status(Status::Code::GOOD), prtcl_ver_e::VERSEOIN_3);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), prtcl_ver_e::VERSEOIN_1);
        }
    }

    jarvis::config::Alarm* ConvertClass::ToAlarmCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::ALARM), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Alarm*>(config);
    }

    jarvis::config::Node* ConvertClass::ToNodeCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::NODE), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Node*>(config);
    }

    jarvis::config::OperationTime* ConvertClass::ToOperationTimeCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::OPERATION_TIME), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::OperationTime*>(config);
    }

    jarvis::config::Production* ConvertClass::ToProductionCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::PRODUCTION_INFO), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Production*>(config);
    }

    jarvis::config::Rs232* ConvertClass::ToRS232CIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::RS232), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Rs232*>(config);
    }

    jarvis::config::Rs485* ConvertClass::ToRS485CIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::RS485), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Rs485*>(config);
    }

    jarvis::config::CatM1* ConvertClass::ToCatM1CIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::LTE_CatM1), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::CatM1*>(config);
    }

    jarvis::config::Ethernet* ConvertClass::ToEthernetCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::ETHERNET), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Ethernet*>(config);
    }

    jarvis::config::WiFi4* ConvertClass::ToWiFi4CIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::WIFI4), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::WiFi4*>(config);
    }

    jarvis::config::Operation* ConvertClass::ToOperationCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::OPERATION), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::Operation*>(config);
    }

    jarvis::config::ModbusRTU* ConvertClass::ToModbusRTUCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::MODBUS_RTU), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::ModbusRTU*>(config);
    }

    jarvis::config::ModbusTCP* ConvertClass::ToModbusTCPCIN(jarvis::config::Base* config)
    {
        ASSERT((config->GetCategory() == jarvis::cfg_key_e::MODBUS_TCP), "CATEGORY DOES NOT MATCH");
        return static_cast<jarvis::config::ModbusTCP*>(config);
    }
}