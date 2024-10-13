/**
 * @file Validator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <WString.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Interfaces/Rs232.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Include/Helper.h"
#include "Validator.h"
#include "Jarvis/Validators/MetaDataValidator.h"
#include "Jarvis/Validators/Interfaces/SerialPortValidator.h"
#include "Jarvis/Validators/Protocol/ModbusValidator.h"
#include "Jarvis/Validators/Network/NetworkValidator.h"


 
namespace muffin { namespace jarvis {

    Validator::Validator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Validator::~Validator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status Validator::Inspect(const JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        ASSERT((mapCIN != nullptr), "OUTPUT PARAMETER <mapCIN> CANNOT BE A NULL POINTER");
        ASSERT((mapCIN->size() == 0), "OUTPUT PARAMETER <mapCIN> MUST BE EMPTY");

        if (jsonDocument.is<JsonObject>() == false)
        {
            LOG_ERROR(logger, "JSON FOR JARVIS MUST HAVE KEY-VALUE PAIRS");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        
        JsonObject json = jsonDocument.as<JsonObject>();
        Status ret = validateMetaData(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID JARVIS METADATA: %s", ret.c_str());
            return ret;
        }
        /*컨테이너 내부 키(key)를 비롯하여 모든 메타데이터는 유효합니다.*/

        ret = emplacePairsForCIN(mapCIN);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO VALIDATE: %s", ret.c_str());
            return Status(Status::Code::BAD_INTERNAL_ERROR);
        }
        /*NULL 값이 아닌 모든 키(key)에 대한 키-값 쌍이 <*mapCIN>에 생성되었습니다.*/

        for (auto pair : json)
        {
            const char* strKey = pair.key().c_str();
            const auto result  = ConvertToConfigKey(mProtocolVersion, strKey);
            ASSERT((result.first.ToCode() == Status::Code::GOOD), "METADATA VALIDATION PROCESS HAS BEEN COMPROMISED");

            const cfg_key_e cinKey    = result.second;
            const JsonArray cinArray  = pair.value().as<JsonArray>();
            cin_vector& outputVector  = mapCIN->at(cinKey);

            switch (cinKey)
            {
                case cfg_key_e::RS232:
                case cfg_key_e::RS485:
                    ret = validateSerialPort(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::WIFI4:
                case cfg_key_e::ETHERNET:
                    ret = validateNicLAN(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::LTE_CatM1:
                    ret = validateNicLTE(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::MODBUS_RTU:
                case cfg_key_e::MODBUS_TCP:
                    ret = validateModbus(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::OPERATION:
                    ret = validateOperation(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::NODE:
                    ret = validateNode(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::ALARM:
                    ret = validateAlarm(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::OPERATION_TIME:
                    ret = validateOperationTime(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::PRODUCTION_INFO:
                    ret = validateProductionInfo(cinKey, cinArray, &outputVector);
                    break;
                default:
                    ASSERT(false, "UNDEFINED CIN KEY");
                    break;
            }

            if (ret != Status::Code::GOOD)
            {
                JsonDocument doc;
                doc.add(pair);

                std::string payload;
                serializeJson(doc, payload);
                
                LOG_ERROR(logger, "INVALID CONFIG INSTANCE: %s: %s", strKey, payload.c_str());
                mapCIN->clear();

                return ret;
            }
        }
    }

    Status Validator::emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        Status ret(Status::Code::UNCERTAIN);
        std::exception exception;
        cfg_key_e cinKey;

        for (const auto key : mContainerKeySet)
        {
            try
            {
                auto result = mapCIN->emplace(key, std::vector<config::Base*>());
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
                cinKey = key;
            }
            catch(const std::bad_alloc& e)
            {
                ret = Status::Code::BAD_OUT_OF_MEMORY;
                exception = e;
                goto ON_FAIL;
            }
            catch(const std::exception& e)
            {
                ret = Status::Code::BAD_UNEXPECTED_ERROR;
                exception = e;
                goto ON_FAIL;
            }
        }

        LOG_VERBOSE(logger, "Emplaced pairs for CIN map");
        return Status(Status::Code::GOOD);
    
    ON_FAIL:
        LOG_ERROR(logger, "%s: CIN class: %s", exception.what(), ConverKeyToString(cinKey));
        mapCIN->clear();
        return ret;
    }

    Status Validator::validateMetaData(const JsonObject json)
    {
        MetaDataValidator validator;
        Status ret = validator.Inspect(json);
        if (ret != Status(Status::Code::GOOD))
        {
            return ret;
        }

        mProtocolVersion   = validator.RetrieveProtocolVersion();
        mRequestIdentifier = validator.RetrieveRequestID();
        mContainerKeySet   = validator.RetrieveContainerKeys();
    
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Protocol Version: v%u", static_cast<uint8_t>(mProtocolVersion));
        LOG_DEBUG(logger, "Request Identifier: %s", mRequestIdentifier);
        for (const auto& key : mContainerKeySet)
        {
            LOG_DEBUG(logger, "Key: %s", ConverKeyToString(key));
        }
    #endif

        return ret;
    }

    Status Validator::validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        SerialPortValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "INVALID SERIAL PORT CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Serial Port Config: %s", ret.c_str());
        }
        return ret;
    }
}}