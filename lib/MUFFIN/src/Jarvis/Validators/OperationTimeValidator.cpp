/**
 * @file OperationTimeValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 가동시간 정보를 수집하기 위한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-10
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

    Status OperationTimeValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        JsonObject json = arrayCIN[0].as<JsonObject>();
        const bool isTotalNull  = json["tot"].isNull();
        const bool isGoodNull   = json["ok"].isNull();
        const bool isDefectNull = json["ng"].isNull();
        if (isTotalNull && isGoodNull && isDefectNull)
        {
            LOG_ERROR(logger, "AT LEAST ONE KEY MUST NOT BE A NULL VALUE");
            return Status(Status::Code::BAD_NO_DATA_AVAILABLE);
        }
        /*적어도 하나의 Node ID가 존재합니다.*/
    }
}}