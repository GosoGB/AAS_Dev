/**
 * @file JSON.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 정의합니다.
 * 
 * @date 2024-09-27
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JSON.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/MacAddress/MacAddress.h"



namespace muffin {

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
            LOG_DEBUG(logger, "Deserialized the payload");
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


    std::string JSON::Serialize(jarvis_struct_t& _struct)
    {
        JsonDocument doc;
        doc["ts"]  = _struct.SourceTimestamp;
        doc["rsc"] = _struct.ResponseCode;
        doc["dsc"] = _struct.Description; 

        JsonArray keyWithNG = doc["cfg"].to<JsonArray>();
        for (auto& key : _struct.Config)
        {
            keyWithNG.add(key);
        }
        
        std::string payload;
        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const remote_controll_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["id"]  = _struct.ID;
        doc["ts"]  = _struct.SourceTimestamp;
        doc["rsc"] = _struct.ResponseCode;
        doc["req"] = _struct.RequestData;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const daq_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["mac"]   =  MacAddress::GetEthernet();
        doc["ts"]    =  _struct.SourceTimestamp;
        doc["name"]  =  _struct.Name;
        doc["uid"]   =  _struct.Uid;
        doc["unit"]  =  _struct.Unit;
        doc["value"] =  _struct.Value;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const alarm_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["mac"]  =  MacAddress::GetEthernet();
        doc["tp"]   =  _struct.AlarmType;
        doc["ts"]   =  _struct.AlarmStartTime;
        doc["tf"]   =  _struct.AlarmFinishTime;
        doc["name"] =  _struct.Name;
        doc["uid"]  =  _struct.Uid;
        doc["id"]   =  _struct.UUID;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const operation_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["mac"]  =  MacAddress::GetEthernet();
        doc["ts"]   =  _struct.SourceTimestamp;
        doc["stauts"]   =  _struct.Status;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const progix_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["mac"]  =  MacAddress::GetEthernet();
        doc["ts"]   =  _struct.SourceTimestamp;
        doc["value"]   =  _struct.Value;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const push_struct_t& _struct)
    {
        JsonDocument doc;
        std::string payload;

        doc["mac"]  =  MacAddress::GetEthernet();
        doc["name"]   =  _struct.Name;
        doc["ts"]   =  _struct.SourceTimestamp;

        serializeJson(doc,payload);

        return payload;
    }

    std::string JSON::Serialize(const fota_status_t& _struct)
    {
        JsonDocument doc;
        std::string payload;
    
    #if defined(MODLINK_L)
        doc["deviceType"] = "MODLINK-L";
    #elif defined(MODLINK_ML10)
        doc["deviceType"] = "MODLINK-ML10";
    #elif defined(MODLINK_T2)
        doc["deviceType"] = "MODLINK-T2";
    #endif

        doc["mac"]  =  MacAddress::GetEthernet();
        JsonObject mcu1 = doc["mcu1"].to<JsonObject>();
        mcu1["vc"] = _struct.VersionCodeMcu1;  
        mcu1["version"] = _struct.VersionMcu1;
    #if defined(MODLINK_T2)
        JsonObject mcu2 = doc["mcu2"].to<JsonObject>();
        mcu2["vc"] = _struct.VersionCodeMcu2;  
        mcu2["version"] = _struct.VersionMcu2; 
    #endif
    
        serializeJson(doc, payload);
        return payload;
    }
    
    void JSON::Serialize(const req_head_t& msg, const uint8_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]  = msg.Code;
        
        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const resp_head_t& msg, const uint8_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]  = msg.Code;
        doc["s"] = msg.Status;
        
        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const resp_vsn_t& msg, const uint8_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]   = msg.Head.Code;
        doc["s"] = msg.Head.Status;
        
        JsonObject response          = doc["r"].to<JsonObject>();
        response["1"]  = msg.SemanticVersion;
        response["2"]      = msg.VersionCode;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const resp_mem_t& msg, const uint16_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]   = msg.Head.Code;
        doc["s"] = msg.Head.Status;
        
        JsonObject response     = doc["r"].to<JsonObject>();
        response["1"]    = msg.Remained;
        response["2"]    = msg.UsedHeap;
        response["3"]    = msg.UsedStack;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const resp_status_t& msg, const uint8_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]   = msg.Head.Code;
        doc["s"]   = msg.Head.Status;
        doc["r"]   = msg.StatusCode;

        serializeJson(doc, output, size);
    }


    void JSON::Serialize(const spear_remote_control_msg_t& msg, const uint8_t size, char output[])
    {
        JsonDocument doc;

        doc["c"]  = msg.Head.Code;
        
        JsonObject body = doc["b"].to<JsonObject>();
        body["1"]   = static_cast<uint8_t>(msg.Link);
        body["2"]   = static_cast<uint8_t>(msg.SlaveID);
        body["3"]   = static_cast<uint8_t>(msg.Area);
        body["4"]   = static_cast<uint16_t>(msg.Address);
        body["5"]   = static_cast<uint16_t>(msg.Value);
        
        serializeJson(doc, output, size);
    }
}