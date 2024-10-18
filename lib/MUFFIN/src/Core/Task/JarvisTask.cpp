/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2024-10-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) EdgecrBoss Inc. 2024
 */





#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "DataFormat/JSON/JSON.h"
#include "Jarvis/Jarvis.h"
#include "Jarvis/Include/Helper.h"
#include "JarvisTask.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "IM/MacAddress/MacAddress.h"



namespace muffin {

    mqtt::Message JarvisTask(JsonDocument& json)
    {
        Jarvis jarvis;
        jarvis::ValidationResult result = jarvis.Validate(json);
        
        const jarvis::rsc_e rsc = result.GetRSC();
        const std::string dsc = result.GetDescription();
        const std::vector<jarvis::cfg_key_e> vectorNgKeys = result.RetrieveKeyWithNG();

        std::string payload;
        JsonDocument doc;
        JsonArray array = doc.createNestedArray("cfg");

        doc["ts"] = GetTimestampInMillis();
        doc["rsc"] = static_cast<uint16_t>(rsc);
        doc["dsc"] = dsc;

        if (vectorNgKeys.size() > 0)
        {
            for (auto key : vectorNgKeys)
            {
                array.add(ConverKeyToString(key));
            }
        }

        serializeJson(doc, payload);
        LOG_DEBUG(logger, "JARVIS Response: %s", payload.c_str());

        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
        return message;
    }

    JsonDocument FetchJarvis()
    {
        JSON json;
        JsonDocument doc;

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, "api.mfm.edgecross.dev", 443, "/api/mfm/device/write", "MODLINK-L/0.0.1");
        http::RequestParameter parameters;
        parameters.AddParameter("mac", MacAddress::GetEthernet());
        Status ret = catHttp.GET(header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH JARVIS FROM SERVER: %s", ret.c_str());
            return doc;
        }
        
        std::string payload;
        ret = catHttp.Retrieve(&payload);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM THE DEVICE");
            return doc;
        }
        
        Status retJSON = json.Deserialize(payload, &doc);
        if (retJSON != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());
            return doc;
        }
        
        return doc;
    }

    std::string CreateDecodingErrorPayload(const char* reason)
    {
        JsonDocument doc;
        doc.createNestedArray("cfg");

        doc["ts"] = GetTimestampInMillis();
        doc["rsc"] = static_cast<uint16_t>(jarvis::rsc_e::BAD_DECODING_ERROR);
        doc["dsc"] = reason;

        std::string payload;
        serializeJson(doc, payload);
        LOG_DEBUG(logger, "JARVIS Response: %s", payload.c_str());
        return payload;
    }
}