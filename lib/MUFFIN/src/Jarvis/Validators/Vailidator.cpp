/**
 * @file Validator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Interfaces/Rs232.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Include/Helper.h"
#include "Jarvis/Validators/Information/AlarmValidator.h"
#include "Jarvis/Validators/Information/NodeValidator.h"
#include "Jarvis/Validators/Information/OperationTimeValidator.h"
#include "Jarvis/Validators/Information/ProductionInfoValidator.h"
#include "Jarvis/Validators/Interfaces/SerialPortValidator.h"
#include "Jarvis/Validators/MetaDataValidator.h"
#include "Jarvis/Validators/Network/LteValidator.h"
#include "Jarvis/Validators/Network/NetworkValidator.h"
#include "Jarvis/Validators/Operation/OperationValidator.h"
#include "Jarvis/Validators/Protocol/ModbusValidator.h"
#include "Validator.h"


 
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

    ValidationResult Validator::Inspect(JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        ASSERT((mapCIN != nullptr), "OUTPUT PARAMETER <mapCIN> CANNOT BE A NULL POINTER");
        ASSERT((mapCIN->size() == 0), "OUTPUT PARAMETER <mapCIN> MUST BE EMPTY");

        ValidationResult result;

        if (jsonDocument.is<JsonObject>() == false)
        {
            mResponseDescription = "BAD: JSON DOCUMENT IS NOT JSON OBJECT";
            LOG_ERROR(logger, "%s", mResponseDescription.c_str());

            result.SetRSC(rsc_e::BAD);
            result.SetDescription(mResponseDescription);
            return result;
        }
        
        JsonObject json = jsonDocument.as<JsonObject>();
        const auto retMetaData = validateMetaData(json);
        if (retMetaData.first > rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            LOG_ERROR(logger, "%s", retMetaData.second.c_str());

            result.SetRSC(retMetaData.first);
            result.SetDescription(retMetaData.second);
            return result;
        }
        else if (retMetaData.first == rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            mPairUncertainRSC = retMetaData;
            LOG_WARNING(logger, "%s", mPairUncertainRSC.second.c_str());
        }
        /*컨테이너 내부 키(key)를 비롯한 메타데이터에 오류는 없습니다만 경고는 있을 수 있습니다.*/

        JsonObject container = json["cnt"];
        LOG_DEBUG(logger, "Container Size: %u", container.size());

        const auto retNotUsedKeys = removeNotUsedKeys(container);
        if (retNotUsedKeys.first != rsc_e::GOOD)
        {
            LOG_ERROR(logger, retNotUsedKeys.second.c_str());

            result.SetRSC(retNotUsedKeys.first);
            result.SetDescription(retNotUsedKeys.second);
            return result;
        }
        LOG_DEBUG(logger, "Container Size: %u", container.size());
        /*사용자가 설정하지 않은 설정에 대한 키-값 쌍이 JSON 개체로부터 모두 제거되었습니다.*/

        const auto retEmplace = emplacePairsForCIN(mapCIN);
        if (retEmplace.first != rsc_e::GOOD)
        {
            LOG_ERROR(logger, "%s", retEmplace.second.c_str());

            result.SetRSC(retEmplace.first);
            result.SetDescription(retEmplace.second);
            return result;
        }
        /*사용자가 설정한 모든 설정에 대한 키-값 쌍이 <*mapCIN>에 생성되었습니다.*/

        for (auto config : container)
        {
            const char* strKey = config.key().c_str();
            const cfg_key_e key = ConvertToConfigKey(mProtocolVersion, strKey).second;
            const JsonArray cinArray = config.value().as<JsonArray>();
            cin_vector& outputVector = mapCIN->at(key);
            std::pair<rsc_e, std::string> ret;

            /**
             * @todo JARVIS 설정 정보 버전에 따라 달라질 수 있는 부분을 고려해야 합니다.
             *       일정 상의 이유로 인해 현재 버전의 코드에는 반영시키지 못하였습니다.
             */
            switch (key)
            {
                case cfg_key_e::RS232:
                case cfg_key_e::RS485:
                    ret = validateSerialPort(key, cinArray, &outputVector);
                    break;
                case cfg_key_e::WIFI4:
                case cfg_key_e::ETHERNET:
                    ret = validateNicLAN(key, cinArray, &outputVector);
                    break;
                case cfg_key_e::LTE_CatM1:
                    ret = validateNicLTE(key, cinArray, &outputVector);
                    break;
                case cfg_key_e::MODBUS_RTU:
                case cfg_key_e::MODBUS_TCP:
                    ret = validateModbus(key, cinArray, &outputVector);
                    break;
                case cfg_key_e::OPERATION:
                    ret = validateOperation(cinArray, &outputVector);
                    break;
                case cfg_key_e::NODE:
                    ret = validateNode(cinArray, &outputVector);
                    break;
                case cfg_key_e::ALARM:
                    ret = validateAlarm(cinArray, &outputVector);
                    break;
                case cfg_key_e::OPERATION_TIME:
                    ret = validateOperationTime(cinArray, &outputVector);
                    break;
                case cfg_key_e::PRODUCTION_INFO:
                    ret = validateProductionInfo(cinArray, &outputVector);
                    break;
                default:
                    ASSERT(false, "UNDEFINED CIN KEY");
                    break;
            }

            try
            {
                mMapRSC.emplace(std::make_pair(key, ret));
            }
            catch(const std::bad_alloc& e)
            {
                result.SetRSC(rsc_e::BAD_OUT_OF_MEMORY);
                result.SetDescription("FAILED TO EMPLACE RESPONSE DATA DUE TO MEMORY ALLOCATION FAILURE");
                return result;
            }
            catch(const std::exception& e)
            {
                result.SetRSC(rsc_e::BAD_UNEXPECTED_ERROR);
                result.SetDescription("FAILED TO EMPLACE RESPONSE DATA WITH UNKNOWN REASON");
                return result;
            }
        }
    
        result = createInspectionReport();
        return result;
    }

    /**
     * @todo 오류 코드 간, 경고 코드 간의 경중을 고려하도록 코드를 수정하는 작업이 필요합니다.
     *       현재 버전의 코드에서는 발견한 첫번째 오류 코드 또는 경고 코드를 서버로 전송합니다.
     *       단, 경고 코드는 오류 코드가 없을 때에만 발생할 수 있습니다.(오류 코드로 덮어씀)
     *       일정 상의 이유로 인해 현재 버전의 코드에는 이러한 부분을 반영시키지 못하였습니다.
     */
    ValidationResult Validator::createInspectionReport()
    {
        ValidationResult result;
        
        for (const auto pair : mMapRSC)
        {
            const cfg_key_e key = pair.first;
            const rsc_e responseCode = pair.second.first;
            const std::string& description = pair.second.second;

            if (responseCode >= rsc_e::BAD)
            {
                result.EmplaceKeyWithNG(key);
                result.SetRSC(responseCode);
                result.SetDescription(description);
                return result;
            }
        }
        /* 오류 코드가 없으니 경고 코드가 있는지 확인합니다. */

        for (const auto pair : mMapRSC)
        {
            const cfg_key_e key = pair.first;
            const rsc_e responseCode = pair.second.first;
            const std::string& description = pair.second.second;

            if (rsc_e::UNCERTAIN <= responseCode && responseCode < rsc_e::BAD)
            {
                result.EmplaceKeyWithNG(key);
                result.SetRSC(responseCode);
                result.SetDescription(description);
                return result;
            }
        }
        /* 설졍 형식에는 오류와 경고 코드가 없습니다. */
    
        if (mIsUncertain == true)
        {/* 메타데이터에 경고가 있었습니다. */
            result.SetRSC(mPairUncertainRSC.first);
            result.SetDescription(mPairUncertainRSC.second);
            return result;
        }
        else
        {
            result.SetRSC(rsc_e::GOOD);
            result.SetDescription("GOOD");
            return result;
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_OUT_OF_MEMORY
     *      @li BAD_UNEXPECTED_ERROR
     */
    std::pair<rsc_e, std::string> Validator::removeNotUsedKeys(JsonObject container)
    {
        std::vector<std::string> vectorNotUsedKeys;
        const uint8_t MAX_KEY_LENGTH = mProtocolVersion == prtcl_ver_e::VERSEOIN_1 ? 12 : 20;
        try
        {
            vectorNotUsedKeys.reserve(MAX_KEY_LENGTH);
        }
        catch(const std::bad_alloc& e)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO RESERVE KEYS UNUSED DUE TO MEMORY ALLOCATION FAILURE");
        }
        catch(const std::exception& e)
        {
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, "FAILED TO RESERVE KEYS UNUSED WITH UNKNOWN REASON");
        }        

        for (auto config : container)
        {
            const char* strKey = config.key().c_str();
            cfg_key_e key = ConvertToConfigKey(mProtocolVersion, strKey).second;
            const bool isNotFound = mContainerKeySet.find(key) == mContainerKeySet.end();
            if (isNotFound == true)
            {
                try
                {
                    vectorNotUsedKeys.emplace_back(std::string(strKey));
                }
                catch(const std::exception& e)
                {
                    return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, "FAILED TO EMPLACE KEY UNUSED WITH UNKNOWN REASON");
                }
            }
        }

        for (auto unusedKey : vectorNotUsedKeys)
        {
            container.remove(unusedKey);
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
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
            for (auto cin : pair.second)
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
        else if (rsc_e::GOOD < ret.first && ret.first < rsc_e::BAD)
        {
            mIsUncertain = true;
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

        if (mIsUncertain == true)
        {
            return mPairUncertainRSC;
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
    }

    std::pair<rsc_e, std::string> Validator::validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        SerialPortValidator validator;
        return validator.Inspect(key, json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateNicLAN(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        NetworkValidator validator;
        return validator.Inspect(key, json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateNicLTE(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        LteValidator validator;
        return validator.Inspect(key, json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateModbus(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        ModbusValidator validator;
        return validator.Inspect(key, json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateOperation(const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        OperationValidator validator;
        return validator.Inspect(json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateNode(const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        NodeValidator validator;
        return validator.Inspect(json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateAlarm(const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        AlarmValidator validator;
        return validator.Inspect(json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateOperationTime(const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        OperationTimeValidator validator;
        return validator.Inspect(json, outputVector);
    }

    std::pair<rsc_e, std::string> Validator::validateProductionInfo(const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        ProductionInfoValidator validator;
        return validator.Inspect(json, outputVector);
    }
}}