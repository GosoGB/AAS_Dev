/**
 * @file JSON.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 정의합니다.
 * 
 * @date 2025-02-10
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JSON.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/Custom/MacAddress/MacAddress.h"



namespace muffin {

    Status JSON::Deserialize(const char* payload, JsonDocument* json)
    {
        ASSERT((strlen(payload) > 0), "INPUT PARAMETER <const char* payload> CANNOT BE EMPTY");
        ASSERT((json != nullptr), "OUTPUT PARAMETER <JsonDocument* json> CANNOT BE A NULL POINTER");
        ASSERT((json->isNull() == true), "OUTPUT PARAMETER <JsonDocument* json> MUST BE EMPTY");

        const DeserializationError error = ArduinoJson::deserializeJson(*json, payload);
        ASSERT((json->isNull() == false), "DESERIALIZED JSON CANNOT BE NULL");
        return processErrorCode(error);
    }

    Status JSON::Deserialize(const std::string& payload, JsonDocument* json)
    {
        ASSERT((payload.length() > 0), "INPUT PARAMETER <const std::string& payload> CANNOT BE EMPTY");
        ASSERT((json != nullptr), "OUTPUT PARAMETER <JsonDocument* json> CANNOT BE A NULL POINTER");
        ASSERT((json->isNull() == true), "OUTPUT PARAMETER <JsonDocument* json> MUST BE EMPTY");

        return Deserialize(payload.c_str(), json);
    }

    Status JSON::Deserialize(fs::File& file, JsonDocument* json)
    {
        ASSERT((file != false), "INPUT PARAMETER <fs::File& file> CANNOT BE NULL");
        
        const DeserializationError error = ArduinoJson::deserializeJson(*json, file);
        ASSERT((json->isNull() == false), "DESERIALIZED JSON CANNOT BE NULL");
        return processErrorCode(error);
    }

    Status JSON::processErrorCode(const DeserializationError& errorCode)
    {
        switch (errorCode.code())
        {
        case DeserializationError::Code::Ok:
            // LOG_DEBUG(logger, "Deserialized the payload");
            return Status(Status::Code::GOOD);

        case DeserializationError::Code::EmptyInput:
            LOG_ERROR(logger, "ERROR: EMPTY PAYLOAD");
            return Status(Status::Code::BAD_NO_DATA);
            
        case DeserializationError::Code::IncompleteInput:
            LOG_ERROR(logger, "ERROR: INSUFFICIENT PAYLOAD");
            return Status(Status::Code::BAD_END_OF_STREAM);
        
        case DeserializationError::Code::InvalidInput:
            LOG_ERROR(logger, "ERROR: INVALID ENCODING");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        
        case DeserializationError::Code::NoMemory:
            LOG_ERROR(logger, "ERROR: OUT OF MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        
        case DeserializationError::Code::TooDeep:
            LOG_ERROR(logger, "ERROR: EXCEEDED NESTING LIMIT");
            return Status(Status::Code::BAD_ENCODING_LIMITS_EXCEEDED);

        default:
            ASSERT(false, "UNDEFINED CONDITION: %s", errorCode.c_str());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::string JSON::Serialize(const jarvis_struct_t& _struct)
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

    void JSON::Serialize(const daq_struct_t& msg, const uint16_t size, char output[])
    {
        ASSERT((size >= UINT8_MAX), "OUTPUT BUFFER MUST BE GREATER THAN UINT8 MAX");

        JsonDocument doc;

        doc["mac"]    = macAddress.GetEthernet();
        doc["ts"]     = msg.SourceTimestamp;
        doc["uid"]    = msg.Uid;
        doc["value"]  = msg.Value;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const alarm_struct_t& msg, const uint16_t size, char output[])
    {
        ASSERT((size >= UINT8_MAX), "OUTPUT BUFFER MUST BE GREATER THAN UINT8 MAX");
        
        JsonDocument doc;

        doc["mac"]   = macAddress.GetEthernet();
        doc["tp"]    = msg.AlarmType;
        doc["ts"]    = msg.AlarmStartTime;
        doc["tf"]    = msg.AlarmFinishTime;
        doc["uid"]   = msg.Uid;
        doc["id"]    = msg.UUID;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const operation_struct_t& msg, const uint16_t size, char output[])
    {
        ASSERT((size >= 128), "OUTPUT BUFFER MUST BE GREATER THAN 128");
        
        JsonDocument doc;

        doc["mac"]     = macAddress.GetEthernet();
        doc["ts"]      = msg.SourceTimestamp;
        doc["status"]  = msg.Status;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const progix_struct_t& msg, const uint16_t size, char output[])
    {
        ASSERT((size >= 128), "OUTPUT BUFFER MUST BE GREATER THAN 128");
        
        JsonDocument doc;

        doc["mac"]    = macAddress.GetEthernet();
        doc["ts"]     = msg.SourceTimestamp;
        doc["value"]  = msg.Value;

        serializeJson(doc, output, size);
    }

    void JSON::Serialize(const push_struct_t& msg, const uint16_t size, char output[])
    {
        ASSERT((size >= 128), "OUTPUT BUFFER MUST BE GREATER THAN 128");
        
        JsonDocument doc;

        doc["mac"]   = macAddress.GetEthernet();
        doc["ts"]    = msg.SourceTimestamp;

        serializeJson(doc, output, size);
    }

    std::string JSON::Serialize(const jarvis_interface_struct_t& _struct)
    {// 512 bytes
        JsonDocument doc;
        std::string payload;

        doc["ts"] =  _struct.SourceTimestamp;
        JsonArray ifArray = doc["if"].to<JsonArray>();
        JsonObject interface = ifArray.add<JsonObject>();

        switch (_struct.SNIC)
        {
        case jvs::snic_e::LTE_CatM1:
            interface["snic"] = "lte";
            break;
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        case jvs::snic_e::Ethernet:
            interface["snic"] = "eth";
            break;
    #endif 
        default:
            interface["snic"] = "undefined";
            break;
        }

        if (_struct.RS485.size() != 0)
        {
            JsonArray rs485 = interface["rs485"].to<JsonArray>();

            for (auto& rs485CIN : _struct.RS485)
            {
                JsonObject _rs485 = rs485.add<JsonObject>();
                _rs485["prt"] = static_cast<uint8_t>(rs485CIN.PortIndex);
                _rs485["bdr"] = static_cast<uint32_t>(rs485CIN.BaudRate);
                _rs485["dbit"] = static_cast<uint8_t>(rs485CIN.DataBit);
                _rs485["pbit"] = static_cast<uint8_t>(rs485CIN.ParityBit);
                _rs485["sbit"] = static_cast<uint8_t>(rs485CIN.StopBit);
            }
            
        }
        
        if (_struct.CatM1.IsCatM1Set == true)
        {
            JsonArray catm1 = interface["catm1"].to<JsonArray>();
            JsonObject _catm1 = catm1.add<JsonObject>();
            _catm1["md"]    = _struct.CatM1.Model == jvs::md_e::LM5 ? "LM5" : "LCM300";
            _catm1["ctry"]  = _struct.CatM1.Country == jvs::ctry_e::KOREA ? "KR" : "USA";
        }
        
        if (_struct.Ethernet.IsEthernetSet == true)
        {
            JsonArray eth   = interface["eth"].to<JsonArray>();
            JsonObject _eth = eth.add<JsonObject>();
            _eth["dhcp"]    = _struct.Ethernet.EnableDHCP;
            
            if (_struct.Ethernet.EnableDHCP == true)
            {
                _eth["ip"]   = nullptr;
                _eth["snm"]  = nullptr;
                _eth["gtw"]  = nullptr;
                _eth["dns1"] = nullptr;
                _eth["dns2"] = nullptr;
            }
            else
            {
                _eth["ip"]   = _struct.Ethernet.StaticIPv4;
                _eth["snm"]  = _struct.Ethernet.Subnetmask;
                _eth["gtw"]  = _struct.Ethernet.Gateway;
                _eth["dns1"] = _struct.Ethernet.DNS1;
                _eth["dns2"] = _struct.Ethernet.DNS2;
            }
            
        }

        serializeJson(doc,payload);

        return payload;
    }

    size_t JSON::Serialize(const fota_status_t& _struct, const size_t size, char output[])
    {
        ASSERT((strlen(output) == 0), "OUTPUT BUFFER MUST BE EMPTY");

        JsonDocument doc;
    #if defined(MODLINK_L)
        doc["deviceType"] = "MODLINK-L";
    #elif defined(MODLINK_ML10)
        doc["deviceType"] = "MODLINK-ML10";
    #elif defined(MODLINK_T2)
        doc["deviceType"] = "MODLINK-T2";
    #endif
        doc["mac"]  =  macAddress.GetEthernet();
        JsonObject mcu1 = doc["mcu1"].to<JsonObject>();
        mcu1["vc"] = _struct.VersionCodeMcu1;  
        mcu1["version"] = _struct.VersionMcu1;
    #if defined(MODLINK_T2)
        JsonObject mcu2 = doc["mcu2"].to<JsonObject>();
        mcu2["vc"] = _struct.VersionCodeMcu2;  
        mcu2["version"] = _struct.VersionMcu2; 
    #endif
    
        return serializeJson(doc, output, size);
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