/**
 * @file Validator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 정의합니다.
 * 
 * @date 2024-10-14
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

    std::pair<rsc_e, std::string> Validator::Inspect(const JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        ASSERT((mapCIN != nullptr), "OUTPUT PARAMETER <mapCIN> CANNOT BE A NULL POINTER");
        ASSERT((mapCIN->size() == 0), "OUTPUT PARAMETER <mapCIN> MUST BE EMPTY");

        if (jsonDocument.is<JsonObject>() == false)
        {
            mResponseDescription = "BAD: JSON DOCUMENT IS NOT JSON OBJECT";
            LOG_ERROR(logger, "%s", mResponseDescription.c_str());
            return std::make_pair(rsc_e::BAD, mResponseDescription);
        }
        
        JsonObject json = jsonDocument.as<JsonObject>();
        const auto retMetaData = validateMetaData(json);
        if (retMetaData.first > rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            LOG_ERROR(logger, "%s", retMetaData.second.c_str());
            return retMetaData;
        }
        /*컨테이너 내부 키(key)를 비롯하여 모든 메타데이터는 유효합니다.*/

        const auto retEmplace = emplacePairsForCIN(mapCIN);
        if (retEmplace.first != rsc_e::GOOD)
        {
            LOG_ERROR(logger, "%s", retEmplace.second.c_str());
            return retEmplace;
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

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_OUT_OF_MEMORY
     *      @li BAD_UNEXPECTED_ERROR
     */
    std::pair<rsc_e, std::string> Validator::emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        std::pair<rsc_e, std::string> ret;

        for (const auto key : mContainerKeySet)
        {
            try
            {
                const auto result = mapCIN->emplace(key, std::vector<config::Base*>());
                ASSERT((result.second == true),
                    "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE"
                );
            }
            catch(const std::bad_alloc& e)
            {
                ret = std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, e.what());
                goto ON_FAIL;
            }
            catch(const std::exception& e)
            {
                ret = std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, e.what());
                goto ON_FAIL;
            }
        }

        ret = std::make_pair(rsc_e::GOOD, "GOOD: Emplaced key-value pairs to CIN map");
        return ret;
    
    ON_FAIL:
        /**
         * @todo 동적으로 할당 된 CIN 개체들이 모두 정상적으로 소멸되었는지
         *       확인해야 합니다. 정상적인 경우 개체는 소멸 로그를 남깁니다.
         */
        for (auto& pair : *mapCIN)
        {
            for (auto& cin : pair.second)
            {
                delete cin;
            }
        }
        
        mapCIN->clear();
        return ret;
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD
     *      @li BAD_INVALID_VERSION
     *      @li BAD_INVALID_VERSION_CONFIG_INSTANCES
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCES
     *      @li BAD_INTERNAL_ERROR
     *      @li UNCERTAIN_VERSION_CONFIG_INSTANCES
     */
    std::pair<rsc_e, std::string> Validator::validateMetaData(const JsonObject json)
    {
        MetaDataValidator validator;
        const auto ret = validator.Inspect(json);
        if (ret.first > rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            return ret;
        }

        const auto retVersion = validator.RetrieveProtocolVersion();
        if (retVersion.first.ToCode() == Status::Code::GOOD)
        {
            mProtocolVersion = retVersion.second;
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "FAILED TO RETRIEVE VERSION INFO");
        }
        
        const auto retRequestID = validator.RetrieveRequestID();
        if (retRequestID.first.ToCode() == Status::Code::GOOD)
        {
            mRequestIdentifier = retRequestID.second;
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "FAILED TO RETRIEVE REQUEST ID");
        }
        
        const auto retContainerKeys = validator.RetrieveContainerKeys();
        if (retContainerKeys.first.ToCode() == Status::Code::GOOD)
        {
            /**
             * @todo std::pair 자료구조로 복사하여 값으로 전달할 때에도
             *       MetaDataValidator의 멤버 변수 mContainerKeySet의
             *       소유권이 이동되었는지 확인해야 합니다.
             */
            mContainerKeySet = std::move(retContainerKeys.second);
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "FAILED TO RETRIEVE CONTAINER KEYS");
        }
    
        if (rsc_e::GOOD < ret.first && ret.first < rsc_e::BAD)
        {
            mIsUncertain = true;
            mPairUncertainRSC = ret;
        }
        return ret;
    }

    std::pair<rsc_e, std::string> Validator::validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        SerialPortValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        switch (ret.ToCode())
        {
        case Status::Code::GOOD:
            return std::make_pair(rsc_e::GOOD, ret.c_str());
        case Status::Code::BAD_NOT_SUPPORTED:
            /* code */
            break;
        
        default:
            break;
        }
        if (ret != Status(Status::Code::GOOD))
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, ret.c_str());
            LOG_ERROR(logger, "INVALID SERIAL PORT CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Serial Port Config: %s", ret.c_str());
        }
        return ret;
    }

    rsc_e Validator::validateNicLAN(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        NetworkValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "INVALID NETWORK CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Network Config: %s", ret.c_str());
        }
        return ret;
    }

    rsc_e Validator::validateNicLTE(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        NetworkValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "INVALID NETWORK CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Network Config: %s", ret.c_str());
        }
        return ret;
    }
}}