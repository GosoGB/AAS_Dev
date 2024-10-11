/**
 * @file OperationTimeValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동시간 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-11
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "OperationTimeValidator.h"
#include "Jarvis/Config/Information/OperationTime.h"



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
     * @brief 
     * 
     * @param key 
     * @param arrayCIN 
     * @param outVector 
     * @return Status 
     *      @li Status::Code::UNCERTAIN 매개변수 <arrayCIN> 중에서 첫번째 CIN 값만 적용됩니다.
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
        mConfigArraySize = arrayCIN.size();
        JsonObject json = arrayCIN[0].as<JsonObject>();
        
        if (json.containsKey("nodeId") == false ||
            json.containsKey("type")   == false ||
            json.containsKey("crit")   == false ||
            json.containsKey("op")     == false)
        {
            LOG_ERROR(logger, "THERE IS MORE THAN ONE MISSING KEY");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        /*모든 키가 존재합니다.*/

        const bool isNodeIdNull      = json["nodeId"].isNull();
        const bool isTypeNull        = json["type"].isNull();
        const bool isCriterionNull   = json["crit"].isNull();
        const bool isOperatorNull    = json["op"].isNull();
        if (isNodeIdNull == true || isTypeNull == true)
        {
            LOG_ERROR(logger, "NODE ID AND TYPE KEYS CANNOT BE NULL");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        /*입력이 필수인 키에 값이 존재합니다.*/

        const std::string nodeID = json["nodeId"].as<std::string>();
        if (nodeID.length()  != 4)
        {
            LOG_ERROR(logger, "NODE ID LENGTH MUST BE EQUAL TO 4");
            return Status(Status::Code::BAD_NODE_ID_INVALID);
        }
        /*참조 대상이 되는 Node ID 값이 유효합니다.*/

        const uint8_t intType = json["type"].as<uint8_t>();
        op_time_type_e type = op_time_type_e::FROM_MACHINE;
        switch (intType)
        {
        case static_cast<uint8_t>(op_time_type_e::FROM_MACHINE):
            type = op_time_type_e::FROM_MACHINE;
            break;
        case static_cast<uint8_t>(op_time_type_e::FROM_MODLINK):
            type = op_time_type_e::FROM_MODLINK;
            break;
        default:
            LOG_ERROR(logger, "INVALID OPERATION TIME TYPE: %u", intType);
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        LOG_VERBOSE(logger, "NODE ID: %s,  Type: %s", nodeID.c_str(),  type == op_time_type_e::FROM_MACHINE ? "Machine" : "MODLINK");
        /*가동시간 유형 값이 유효합니다. (리눅스 한글 입력 오타인 듯)*/
        /*The type of operation time is valid*/
        
        if (type == op_time_type_e::FROM_MACHINE)
        {
            config::OperationTime* opTime = new config::OperationTime(key);
            opTime->SetNodeID(nodeID);
            opTime->SetType(type);
        }
    }
}}