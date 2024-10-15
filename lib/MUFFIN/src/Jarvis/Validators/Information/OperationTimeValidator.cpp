/**
 * @file OperationTimeValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동시간 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-11
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Information/OperationTime.h"
#include "OperationTimeValidator.h"



namespace muffin { namespace jarvis {    

    OperationTimeValidator::OperationTimeValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    OperationTimeValidator::~OperationTimeValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    /**
     * @return Status 
     *      @li Status::Code::UNCERTAIN 매개변수 <arrayCIN> 중에서 첫번째 CIN 값만 적용됐습니다.
     */
    std::pair<rsc_e, std::string> OperationTimeValidator::Inspect(const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

        /**
         * @todo 현재는 한 번에 하나의 설정만 받는 것을 염두에 둔 설계입니다.
         *       다만, 향후에 multi-drop 방식과 같이 두 개 이상의 슬레이브와
         *       연동하는 경우에는 두 개 이상의 설정이 필요할 수 있습니다.
         * 
         *       그러한 상황이 된다면 하나의 설정만 받는 현행 방식의 코드를
         *       두 개 이상의 설정을 받을 수 있도록 수정해야 합니다.
         */
        JsonObject json = arrayCIN[0].as<JsonObject>();
        
        rsc_e rsc = validateMandatoryKeys(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION TIME: MANDATORY KEY CANNOT BE MISSING");
        }

        rsc = validateMandatoryValues(json);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID OPERATION TIME: MANDATORY KEY'S VALUE CANNOT BE NULL");
        }

        const std::string nodeID = json["nodeId"].as<std::string>();
        if (nodeID.length()  != 4)
        {
            const std::string message = "NODE ID LENGTH MUST BE EQUAL TO 4 NODEID: " + nodeID;
            return std::make_pair(rsc, message);
        }

        const uint8_t type = json["type"].as<uint8_t>();
        const auto retType = convertToOperationTimeType(type);
        if (retType.first != rsc_e::GOOD)
        {
            const std::string message = "INVALID OPERATION TIME TYPE:" + std::to_string(type);
            return std::make_pair(rsc, message);
        }

        config::OperationTime* operationTime = new(std::nothrow) config::OperationTime();
        if (operationTime == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR OPERATION TIME CONFIG");
        }
        
        operationTime->SetNodeID(nodeID);
        operationTime->SetType(retType.second);

        if (retType.second != op_time_type_e::FROM_MACHINE)
        {
            const bool isCriterionNull  = json["crit"].isNull();
            const bool isOperatorNull   = json["op"].isNull();
            if (isCriterionNull == true || isOperatorNull == true)
            {
                const std::string message = "CRITERION AND OPERATOR KEYS CANNOT BE NULL WHEN TYPE IS 1";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            const bool isCriterionInteger = json["crit"].is<int32_t>();
            if (isCriterionInteger == false)
            {
                const std::string message = "INVALID CRITERION: NOT A 32-BIT INTEGER";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            const int32_t criterion = json["crit"].as<int32_t>();

            const std::string stringOperator = json["op"].as<std::string>();
            const auto retOperator = convertToLogicalOperator(stringOperator);
            if (retOperator.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID LOGICAL OPERATOR:" + stringOperator;
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            operationTime->SetCriterion(criterion);
            operationTime->SetOperator(retOperator.second);
        }

        rsc = emplaceCIN(static_cast<config::Base*>(operationTime), outVector);
        if (rsc != rsc_e::GOOD)
        {
            if (operationTime != nullptr)
            {
                delete operationTime;
                operationTime = nullptr;
            }
            return std::make_pair(rsc, "FAILED TO EMPLACE: OPERATION TIME CONFIG INSTANCE");
        }
        
        if (arrayCIN.size() > 1)
        {
            const std::string message = "ONLY ONE OPERATION TIME INFO CONFIG WILL BE APPLIED";
            return std::make_pair(rsc_e::UNCERTAIN, message);
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
    }

    rsc_e OperationTimeValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("nodeId");
        isValid &= json.containsKey("type");
        isValid &= json.containsKey("crit");
        isValid &= json.containsKey("op");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e OperationTimeValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["nodeId"].isNull()  == false;
        isValid &= json["type"].isNull()  == false;
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e OperationTimeValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
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

    std::pair<rsc_e, op_time_type_e> OperationTimeValidator::convertToOperationTimeType(const uint8_t type)
    {
        switch (type)
        {
        case 1:
            return std::make_pair(rsc_e::GOOD, op_time_type_e::FROM_MACHINE);
        case 2:
            return std::make_pair(rsc_e::GOOD, op_time_type_e::FROM_MODLINK);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, op_time_type_e::FROM_MACHINE);
        }
    }

    std::pair<rsc_e, cmp_op_e> OperationTimeValidator::convertToLogicalOperator(const std::string& stringOperator)
    {
        if (stringOperator == "<")
        {
            return std::make_pair(rsc_e::GOOD, cmp_op_e::LESS_THAN);
        }
        else if (stringOperator == "<=")
        {
            return std::make_pair(rsc_e::GOOD, cmp_op_e::LESS_EQUAL);
        }
        else if (stringOperator == "==")
        {
            return std::make_pair(rsc_e::GOOD, cmp_op_e::EQUAL);
        }
        else if (stringOperator == ">=")
        {
            return std::make_pair(rsc_e::GOOD, cmp_op_e::GREATER_EQUAL);
        }
        else if (stringOperator == ">")
        {
            return std::make_pair(rsc_e::GOOD, cmp_op_e::GREATER_THAN);
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, cmp_op_e::LESS_THAN);
        }
    }
}}