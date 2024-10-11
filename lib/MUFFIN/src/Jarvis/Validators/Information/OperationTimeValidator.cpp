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
    Status OperationTimeValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((key == cfg_key_e::OPERATION_TIME), "CONFIG CATEGORY DOES NOT MATCH");

        /**
         * @todo 현재는 한 번에 하나의 설정만 받는 것을 염두에 둔 설계입니다.
         *       다만, 향후에 multi-drop 방식과 같이 두 개 이상의 슬레이브와
         *       연동하는 경우에는 두 개 이상의 설정이 필요할 수 있습니다.
         * 
         *       그러한 상황이 된다면 하나의 설정만 받는 현행 방식의 코드를
         *       두 개 이상의 설정을 받을 수 있도록 수정해야 합니다.
         */
        JsonObject json = arrayCIN[0].as<JsonObject>();
        
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
        if (nodeID.length()  != 4)
        {
            LOG_ERROR(logger, "NODE ID LENGTH MUST BE EQUAL TO 4");
            return Status(Status::Code::BAD_NODE_ID_INVALID);
        }

        const uint8_t type = json["type"].as<uint8_t>();
        const auto retType = convertToOperationTimeType(type);
        if (retType.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID OPERATION TIME TYPE: %u", type);
            return retType.first;
        }

        config::OperationTime* operationTime = new(std::nothrow) config::OperationTime(cfg_key_e::OPERATION_TIME);
        if (operationTime == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: OPERATION TIME");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        operationTime->SetNodeID(nodeID);
        operationTime->SetType(retType.second);

        if (retType.second != op_time_type_e::FROM_MACHINE)
        {
            const bool isCriterionNull  = json["crit"].isNull();
            const bool isOperatorNull   = json["op"].isNull();
            if (isCriterionNull == true || isOperatorNull == true)
            {
                LOG_ERROR(logger, "CRITERION AND OPERATOR KEYS CANNOT BE NULL WHEN TYPE IS 1");
                return Status(Status::Code::BAD_ENCODING_ERROR);
            }

            const bool isCriterionInteger = json["crit"].is<int32_t>();
            if (isCriterionInteger == false)
            {
                LOG_ERROR(logger, "INVALID CRITERION: NOT A 32-BIT INTEGER");
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }
            const int32_t criterion = json["crit"].as<int32_t>();

            const std::string stringOperator = json["op"].as<std::string>();
            const auto retOperator = convertToLogicalOperator(stringOperator);
            if (retOperator.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID LOGICAL OPERATOR: %s", stringOperator.c_str());
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }

            operationTime->SetCriterion(criterion);
            operationTime->SetOperator(retOperator.second);
        }

        ret = emplaceCIN(static_cast<config::Base*>(operationTime), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE OPERATION TIME CIN: %s", ret.c_str());
            delete operationTime;
            return ret;
        }
        
        if (arrayCIN.size() > 1)
        {
            LOG_WARNING(logger, "ONLY ONE OPERATION TIME INFO CONFIG WILL BE APPLIED");
            return Status(Status::Code::UNCERTAIN);
        }
        else
        {
            return ret;
        }
    }

    Status OperationTimeValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("nodeId");
        isValid &= json.containsKey("type");
        isValid &= json.containsKey("crit");
        isValid &= json.containsKey("op");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status OperationTimeValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["nodeId"].isNull()  == false;
        isValid &= json["type"].isNull()  == false;
        
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status OperationTimeValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
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

    std::pair<Status, op_time_type_e> OperationTimeValidator::convertToOperationTimeType(const uint8_t type)
    {
        switch (type)
        {
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), op_time_type_e::FROM_MACHINE);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), op_time_type_e::FROM_MODLINK);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), op_time_type_e::FROM_MACHINE);
        }
    }

    std::pair<Status, cmp_op_e> OperationTimeValidator::convertToLogicalOperator(const std::string& stringOperator)
    {
        cmp_op_e logicalOperator = cmp_op_e::LESS_THAN;

        if (stringOperator == "<")
        {
            logicalOperator = cmp_op_e::LESS_THAN;
        }
        else if (stringOperator == "<=")
        {
            logicalOperator = cmp_op_e::LESS_EQUAL;
        }
        else if (stringOperator == "==")
        {
            logicalOperator = cmp_op_e::EQUAL;
        }
        else if (stringOperator == ">=")
        {
            logicalOperator = cmp_op_e::GREATER_EQUAL;
        }
        else if (stringOperator == ">")
        {
            logicalOperator = cmp_op_e::GREATER_THAN;
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), logicalOperator);
        }

        return std::make_pair(Status(Status::Code::GOOD), logicalOperator);
    }
}}