/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 전반에 걸쳐 공통으로 사용할 수 있는 함수를 정의합니다.
 * 
 * @date 2024-10-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <errno.h>
#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Helper.h"



namespace muffin { namespace jarvis {

    uint8_t ConvertToUInt8(const char* str)
    {
        char* endPointer;
        errno = 0;

        uint32_t value = strtoul(str, &endPointer, 10);
        
        if (endPointer == str)
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", str);
            return UINT8_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", endPointer);
            return UINT8_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", str);
            return UINT8_MAX;
        }
        else
        {
            return value;
        }
    }

    const char* ConverKeyToString(const cfg_key_e key)
    {
        switch (key)
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
            ASSERT(false, "UNDEFINED CONFIG INSTANCE KEY");
            return nullptr;
        }
    }



    std::pair<Status, cfg_key_e> ConvertToConfigKey(const prtcl_ver_e version, const char* str)
    {
        if (static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_1) <= static_cast<uint8_t>(version))
        {
            if (strcmp(str, "rs232") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::RS232);
            }
            else if (strcmp(str, "rs485") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::RS485);
            }
            else if (strcmp(str, "wifi") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::WIFI4);
            }
            else if (strcmp(str, "eth") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::ETHERNET);
            }
            else if (strcmp(str, "catm1") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::LTE_CatM1);
            }
            else if (strcmp(str, "mbrtu") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::MODBUS_RTU);
            }
            else if (strcmp(str, "mbtcp") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::MODBUS_TCP);
            }
            else if (strcmp(str, "op") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::OPERATION);
            }
            else if (strcmp(str, "node") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::NODE);
            }
            else if (strcmp(str, "alarm") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::ALARM);
            }
            else if (strcmp(str, "optime") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::OPERATION_TIME);
            }
            else if (strcmp(str, "prod") == 0)
            {
                return std::make_pair(Status(Status::Code::GOOD), cfg_key_e::PRODUCTION_INFO);
            }
            else
            {
                LOG_ERROR(logger, "INVALID KEY: %s", str);
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), cfg_key_e::ALARM);
            }
        }
        else
        {
            ASSERT(false, "UNSUPPORTED VERSION");
            return std::make_pair(Status(Status::Code::BAD_PROTOCOL_VERSION_UNSUPPORTED), cfg_key_e::ALARM);
        }
    }
}}