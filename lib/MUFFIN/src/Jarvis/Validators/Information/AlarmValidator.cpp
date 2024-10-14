/**
 * @file AlarmValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 이벤트를 생성하기 위한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex>

#include "AlarmValidator.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"



namespace muffin { namespace jarvis {  

    AlarmValidator::AlarmValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    AlarmValidator::~AlarmValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status AlarmValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((key == cfg_key_e::OPERATION_TIME), "CONFIG CATEGORY DOES NOT MATCH");

        for (JsonObject json : arrayCIN)
        {
            Status ret = validateMandatoryKeys(json);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MANDATORY KEYS CANNOT BE MISSING");
                return ret;
            }

            ret = validateMandatoryValues(json);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
                return ret;
            }

            const std::string nodeID = json["nodeId"].as<std::string>();
            if (nodeID.length() != 4)
            {
                LOG_ERROR(logger, "NODE ID LENGTH MUST BE EQUAL TO 4");
                return Status(Status::Code::BAD_NODE_ID_INVALID);
            }

            const uint8_t type = json["type"].as<uint8_t>();
            const auto retType = convertToAlarmType(type);
            if (retType.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ALARM TYPE: %u", type);
                return retType.first;
            }

            config::Alarm* alarm = new(std::nothrow) config::Alarm();
            if (alarm == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: ALARM");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            alarm->SetNodeID(nodeID);
            alarm->SetType(retType.second);

            switch (retType.second)
            {
            case alarm_type_e::ONLY_LCL:
                ret = processLowerControlLimit(json, alarm);
                break;
            case alarm_type_e::ONLY_UCL:
                ret = processUpperControlLimit(json, alarm);
                break;
            case alarm_type_e::LCL_AND_UCL:
                ret = processControlLimits(json, alarm);
                break;
            case alarm_type_e::CONDITION:
                ret = processConditions(json, alarm);
                break;
            default:
                ASSERT(false, "UNDEFINED ALARM TYPE");
                break;
            }

            ret = emplaceCIN(static_cast<config::Base*>(alarm), outVector);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE ALARM CIN: %s", ret.c_str());
                delete alarm;
                return ret;
            }
        }

        LOG_VERBOSE(logger, "Valid Alarm config instance");
        return Status(Status::Code::GOOD);
    }

    Status AlarmValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("nodeId");
        isValid &= json.containsKey("type");
        isValid &= json.containsKey("lcl");
        isValid &= json.containsKey("ucl");
        isValid &= json.containsKey("cnd");
        isValid &= json.containsKey("lcl-uid");
        isValid &= json.containsKey("ucl-uid");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status AlarmValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["nodeId"].isNull()  == false;
        isValid &= json["type"].isNull()    == false;
        
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status AlarmValidator::processLowerControlLimit(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isLclNull     = json["lcl"].isNull();
        const bool isLclUidNull  = json["lcl-uid"].isNull();
        if (isLclNull == true || isLclUidNull == true)
        {
            LOG_ERROR(logger, "LCL AND UID KEYS CANNOT BE NULL WHEN TYPE IS 1");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        const bool isFloat = json["lcl"].is<float>();
        if (isFloat == false)
        {
            LOG_ERROR(logger, "INVALID LCL: NOT A 32-BIT FLOATING POINT");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        const int32_t lcl = json["lcl"].as<float>();
        
        const std::string uid = json["lcl-uid"].as<std::string>();
        const std::regex pattern("^P\\d{4}$");
        const bool isUidValid = std::regex_match(uid, pattern);
        if (isUidValid == false)
        {
            LOG_ERROR(logger, "INVALID LCL UID: %s", uid.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        cin->SetLCL(lcl);
        cin->SetLclUID(uid);
        return Status(Status::Code::GOOD);
    }

    Status AlarmValidator::processUpperControlLimit(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isUclNull     = json["ucl"].isNull();
        const bool isUclUidNull  = json["ucl-uid"].isNull();
        if (isUclNull == true || isUclUidNull == true)
        {
            LOG_ERROR(logger, "UCL AND UID KEYS CANNOT BE NULL WHEN TYPE IS 1");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        const bool isFloat = json["ucl"].is<float>();
        if (isFloat == false)
        {
            LOG_ERROR(logger, "INVALID UCL: NOT A 32-BIT FLOATING POINT");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        const int32_t ucl = json["ucl"].as<float>();
        
        const std::string uid = json["ucl-uid"].as<std::string>();
        const std::regex pattern("^P\\d{4}$");
        const bool isUidValid = std::regex_match(uid, pattern);
        if (isUidValid == false)
        {
            LOG_ERROR(logger, "INVALID UCL UID: %s", uid.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        cin->SetUCL(ucl);
        cin->SetUclUID(uid);
        return Status(Status::Code::GOOD);
    }

    Status AlarmValidator::processControlLimits(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        Status ret = processLowerControlLimit(json, cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LOWER CONTROL LIMIT CONFIG");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        ret = processUpperControlLimit(json, cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID UPPER CONTROL LIMIT CONFIG");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        return Status(Status::Code::GOOD);
    }

    Status AlarmValidator::processConditions(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isConditionNull = json["cnd"].isNull();
        const bool isConditionEmpty = json["cnd"].size() == 0;
        if (isConditionNull == true || isConditionEmpty == true)
        {
            LOG_ERROR(logger, "CONDITIONS CANNOT BE NULL OR EMPTY ARRAY");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        std::vector<int16_t> vectorCondition;
        try
        {
            vectorCondition.reserve(json["cnd"].size());
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        JsonArray conditions = json["cnd"].as<JsonArray>();
        for (auto condition : conditions)
        {
            if (condition.is<int16_t>() == false)
            {
                LOG_ERROR(logger, "CONDITION MUST BE 16-BIT INTEGER");
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }

            vectorCondition.emplace_back(condition.as<int16_t>());
        }
        
        cin->SetCondition(vectorCondition);
        return Status(Status::Code::GOOD);
    }

    Status AlarmValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "INPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::pair<Status, alarm_type_e> AlarmValidator::convertToAlarmType(const uint8_t type)
    {
        switch (type)
        {
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), alarm_type_e::ONLY_LCL);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), alarm_type_e::ONLY_UCL);
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), alarm_type_e::LCL_AND_UCL);
        case 4:
            return std::make_pair(Status(Status::Code::GOOD), alarm_type_e::CONDITION);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), alarm_type_e::ONLY_LCL);
        }
    }
}}