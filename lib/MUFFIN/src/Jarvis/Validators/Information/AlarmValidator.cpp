/**
 * @file AlarmValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 이벤트를 생성하기 위한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-12
 * @version 1.0.0
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

    std::pair<rsc_e, std::string> AlarmValidator::Inspect(const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

        for (JsonObject json : arrayCIN)
        {
            rsc_e rsc = validateMandatoryKeys(json);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID ALARM: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValues(json);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID ALARM: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const std::string nodeID = json["nodeId"].as<std::string>();
            if (nodeID.length() != 4)
            {
                const std::string message = "NODE ID LENGTH MUST BE EQUAL TO 4, NODEID: " + nodeID;
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            const uint8_t type = json["type"].as<uint8_t>();
            const auto retType = convertToAlarmType(type);
            if (retType.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID ALARM TYPE: " + std::to_string(type);
                return std::make_pair(retType.first, message);
            }

            config::Alarm* alarm = new(std::nothrow) config::Alarm();
            if (alarm == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR ALARM CONFIG");
            }

            alarm->SetNodeID(nodeID);
            alarm->SetType(retType.second);

            std::pair<rsc_e, std::string> result = std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, "UNDEFINED ALARM TYPE");
            
            switch (retType.second)
            {
            case alarm_type_e::ONLY_LCL:
                result = processLowerControlLimit(json, alarm);
                break;
            case alarm_type_e::ONLY_UCL:
                result = processUpperControlLimit(json, alarm);
                break;
            case alarm_type_e::LCL_AND_UCL:
                result = processControlLimits(json, alarm);
                break;
            case alarm_type_e::CONDITION:
                result = processConditions(json, alarm);
                break;
            default:
                ASSERT(false, "UNDEFINED ALARM TYPE");
                break;
            }
            
            if ( result.first != rsc_e::GOOD)
            {
                return result;
            }
            
            rsc = emplaceCIN(static_cast<config::Base*>(alarm), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (alarm != nullptr)
                {
                    delete alarm;
                    alarm = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: ALARM CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");    
    }

    rsc_e AlarmValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("nodeId");
        isValid &= json.containsKey("type");
        isValid &= json.containsKey("lcl");
        isValid &= json.containsKey("lclUid");
        isValid &= json.containsKey("lclAUid");
        isValid &= json.containsKey("ucl");
        isValid &= json.containsKey("uclUid");
        isValid &= json.containsKey("uclAUid");
        isValid &= json.containsKey("cnd");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e AlarmValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["nodeId"].isNull()  == false;
        isValid &= json["type"].isNull()    == false;
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    std::pair<rsc_e, std::string> AlarmValidator::processLowerControlLimit(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isLclNull          = json["lcl"].isNull();
        const bool isLclUidNull       = json["lclUid"].isNull();
        const bool isLclAlarmUidNull  = json["lclAUid"].isNull();
        if (isLclNull == true || isLclUidNull == true || isLclAlarmUidNull == true)
        {
            const std::string message = "LCL AND UID KEYS CANNOT BE NULL WHEN TYPE IS 1";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        const bool isFloat = json["lcl"].is<float>();
        if (isFloat == false)
        {
            const std::string message = "INVALID LCL: NOT A 32-BIT FLOATING POINT";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        /**
         * @todo 김주성 전임과 float로 변경하는 것에 대해 논의해보기
         */
        const int32_t lcl = json["lcl"].as<float>();
        
        const std::string uid = json["lclUid"].as<std::string>();
        const std::regex patternUID("^P[A-Fa-f0-9!@#$%^&*()_+=-]{3}$");
        const bool isUidValid = std::regex_match(uid, patternUID);
        if (isUidValid == false)
        {
            const std::string message = "INVALID LCL UID: "+ uid;
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        const std::string auid = json["lclAUid"].as<std::string>();
        const std::regex patternAUID("^A[A-Fa-f0-9!@#$%^&*()_+=-]{3}$");
        const bool isAUidValid = std::regex_match(auid, patternAUID);
        if (isAUidValid == false)
        {
            const std::string message = "INVALID LCL ALARM UID: "+ uid;
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        cin->SetLCL(lcl);
        cin->SetLclUID(uid);
        cin->SetLclAlarmUID(auid);

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    std::pair<rsc_e, std::string> AlarmValidator::processUpperControlLimit(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isUclNull          = json["ucl"].isNull();
        const bool isUclUidNull       = json["uclUid"].isNull();
        const bool isUclAlarmUidNull  = json["uclAUid"].isNull();
        if (isUclNull == true || isUclUidNull == true || isUclAlarmUidNull == true)
        {
            const std::string message = "UCL AND UID KEYS CANNOT BE NULL WHEN TYPE IS 1";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        const bool isFloat = json["ucl"].is<float>();
        if (isFloat == false)
        {
            const std::string message = "INVALID UCL: NOT A 32-BIT FLOATING POINT";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        const int32_t ucl = json["ucl"].as<float>();
        
        const std::string uid = json["uclUid"].as<std::string>();
        const std::regex pattern("^P[A-Fa-f0-9!@#$%^&*()_+=-]{3}$");
        const bool isUidValid = std::regex_match(uid, pattern);
        if (isUidValid == false)
        {
            const std::string message = "INVALID UCL UID: "+ uid;
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        const std::string auid = json["uclAUid"].as<std::string>();
        const std::regex patternAUID("^A[A-Fa-f0-9!@#$%^&*()_+=-]{3}$");
        const bool isAUidValid = std::regex_match(auid, patternAUID);
        if (isAUidValid == false)
        {
            const std::string message = "INVALID UCL ALARM UID: "+ uid;
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        cin->SetUCL(ucl);
        cin->SetUclUID(uid);
        cin->SetUclAlarmUID(auid);

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    std::pair<rsc_e, std::string> AlarmValidator::processControlLimits(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");
        std::pair<rsc_e, std::string> rsc = processLowerControlLimit(json, cin);
        
        if (rsc.first != rsc_e::GOOD)
        {
            return rsc;
        }

        rsc = processUpperControlLimit(json, cin);
        if (rsc.first != rsc_e::GOOD)
        {
            return rsc;
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");   
    }

    std::pair<rsc_e, std::string> AlarmValidator::processConditions(const JsonObject json, config::Alarm* cin)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        const bool isConditionNull = json["cnd"].isNull();
        const bool isConditionEmpty = json["cnd"].size() == 0;
        if (isConditionNull == true || isConditionEmpty == true)
        {
            const std::string message = "CONDITIONS CANNOT BE NULL OR EMPTY ARRAY";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        std::vector<int16_t> vectorCondition;
        try
        {
            vectorCondition.reserve(json["cnd"].size());
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "ALLOC ERROR : BAD_OUT_OF_MEMORY");
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, "ALLOC ERROR : BAD_UNEXPECTED_ERROR");
        }

        JsonArray conditions = json["cnd"].as<JsonArray>();
        for (auto condition : conditions)
        {
            if (condition.is<int16_t>() == false)
            {
                const std::string message = "CONDITION MUST BE 16-BIT INTEGER";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            vectorCondition.emplace_back(condition.as<int16_t>());
        }
        
        cin->SetCondition(vectorCondition);
        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    rsc_e AlarmValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "INPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    std::pair<rsc_e, alarm_type_e> AlarmValidator::convertToAlarmType(const uint8_t type)
    {
        switch (type)
        {
        case 1:
            return std::make_pair(rsc_e::GOOD, alarm_type_e::ONLY_LCL);
        case 2:
            return std::make_pair(rsc_e::GOOD, alarm_type_e::ONLY_UCL);
        case 3:
            return std::make_pair(rsc_e::GOOD, alarm_type_e::LCL_AND_UCL);
        case 4:
            return std::make_pair(rsc_e::GOOD, alarm_type_e::CONDITION);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, alarm_type_e::ONLY_LCL);
        }
    }
}}