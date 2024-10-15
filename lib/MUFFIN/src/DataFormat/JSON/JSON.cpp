/**
 * @file JSON.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 정의합니다.
 * 
 * @date 2024-09-27
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JSON.h"



namespace muffin {

    JSON::JSON()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    JSON::~JSON()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status JSON::Deserialize(const std::string& payload, JsonDocument* json)
    {
        ASSERT((payload.length() > 0), "INPUT PARAMETER <const std::string& payload> CANNOT BE EMPTY");
        ASSERT((json != nullptr), "OUTPUT PARAMETER <JsonDocument* json> CANNOT BE A NULL POINTER");
        ASSERT((json->isNull() == true), "OUTPUT PARAMETER <JsonDocument* json> MUST BE EMPTY");

        const DeserializationError error = ArduinoJson::deserializeJson(*json, payload);
        ASSERT((json->isNull() == false), "DESERIALIZED JSON CANNOT BE NULL");

        switch (error.code())
        {
        case DeserializationError::Code::Ok:
            LOG_INFO(logger, "Deserialized the payload");
            return Status(Status::Code::GOOD);

        case DeserializationError::Code::EmptyInput:
            LOG_ERROR(logger, "ERROR: EMPTY PAYLOAD");
            return Status(Status::Code::BAD_NO_DATA);
            
        case DeserializationError::Code::IncompleteInput:
            LOG_ERROR(logger, "ERROR: INSUFFICIENT PAYLOAD: %s", payload.c_str());
            return Status(Status::Code::BAD_END_OF_STREAM);
        
        case DeserializationError::Code::InvalidInput:
            LOG_ERROR(logger, "ERROR: INVALID ENCODING: %s", payload.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        
        case DeserializationError::Code::NoMemory:
            LOG_ERROR(logger, "ERROR: OUT OF MEMORY: %u", payload.size());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        
        case DeserializationError::Code::TooDeep:
            LOG_ERROR(logger, "ERROR: EXCEEDED NESTING LIMIT: %s", payload.c_str());
            return Status(Status::Code::BAD_ENCODING_LIMITS_EXCEEDED);

        default:
            ASSERT(false, "UNDEFINED CONDITION: %s", error.c_str());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }
}