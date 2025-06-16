/**
 * @file SPEAR.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPEAR 프로토콜 클래스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#if defined(MT10) || defined(MB10)

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "SPEAR.h"

namespace muffin {

    SemaphoreHandle_t xSemaphoreSPEAR = NULL;

    Status SPEAR::Init()
    {
        Serial2.begin(115200);
        
        xSemaphoreSPEAR = xSemaphoreCreateMutex();
        if (xSemaphoreSPEAR == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE SPEAR SEMAPHORE");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        return SignOnService();
    }

    Status SPEAR::Reset()
    {
        pinMode(RESET_PIN, OUTPUT);
        
        digitalWrite(RESET_PIN, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        digitalWrite(RESET_PIN, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        digitalWrite(RESET_PIN, HIGH);
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::Send(const char payload[])
    {
        size_t length = HEAD_SIZE + strlen(payload) + TAIL_SIZE;
        char buffer[length] = {'\0'};
        Status ret = buildMessage(payload, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        Serial2.write(buffer, length);
        Serial2.flush();
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::Receive(const uint16_t timeoutMillis, const size_t size, char payload[])
    {
        memset(payload, '\0', size);

        size_t length = 0;
        const uint32_t startedMillis = millis();
    
        while ((millis() - startedMillis) < timeoutMillis) 
        {
            if (Serial2.available() >= HEAD_SIZE) 
            {
                break;
            }
            delay(10);
        }

        if (Serial2.available() < HEAD_SIZE) 
        {
            LOG_ERROR(logger, "TIMEOUT WAITING FOR HEADER");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        while (length < HEAD_SIZE && (millis() - startedMillis) < timeoutMillis) 
        {
            if (Serial2.available()) 
            {
                int rxd = Serial2.read();
                if (rxd == -1)
                {
                    continue;
                }
                payload[length++] = rxd;
            }
        }
        
        // 헤더 검증
        if (payload[0] != STX) 
        {
            LOG_ERROR(logger, "INVALID START BYTE : 0x%02X", payload[0]);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        // 헤더에서 payload 길이(2바이트)를 추출
        const uint16_t receivedPayloadLength = ((uint16_t)payload[1] << 8) | payload[2];
        // 전체 패킷 길이: 헤더 + payload + (체크섬 + ETX)
        const uint16_t totalExpectedLength = HEAD_SIZE + receivedPayloadLength + TAIL_SIZE;
    
        // 전체 패킷 수신
        while ((millis() - startedMillis) < timeoutMillis && length < totalExpectedLength) 
        {
            if (Serial2.available()) 
            {
                int rxd = Serial2.read();
                if (rxd == -1)
                {
                    continue;
                }
                payload[length++] = rxd;
            }
        }

        if (length < totalExpectedLength)
        {
            LOG_ERROR(logger, "TIMEOUT WAITING FOR FULL PACKET, EXPECTED %d BYTES, RECEIVED %d BYTES", totalExpectedLength, length);
            return Status(Status::Code::BAD_TIMEOUT);
        }
    
        if (payload[length - 1] != ETX) 
        {
            LOG_ERROR(logger, "MISSING ETX. FOUND: 0x%02X", payload[length - 1]);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        
        const uint8_t receivedChecksum = payload[length - 2];
    
        // 실제 payload 데이터 추출 (헤더 제거)
        for (uint16_t i = 0; i < receivedPayloadLength; ++i) 
        {
            payload[i] = payload[HEAD_SIZE + i];
        }
        payload[receivedPayloadLength] = '\0';
    
        const uint8_t calculatedChecksum = calculateChecksum(payload);
        if (calculatedChecksum != receivedChecksum) 
        {
            LOG_ERROR(logger, "CHECKSUM ERROR: calculated 0x%02X, received 0x%02X", calculatedChecksum, receivedChecksum);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        
        return Status(Status::Code::GOOD);
    }

    uint8_t SPEAR::calculateChecksum(const char payload[])
    {
        const uint16_t length = strlen(payload);
        const uint8_t lengthHIGH  = (length >> 8) & 0xFF;
        const uint8_t lengthLOW   = length & 0xFF;

        uint8_t checksum = 0x00;
        checksum ^= lengthHIGH;
        checksum ^= lengthLOW;

        for (uint16_t i = 0; i < length; ++i)
        {
            checksum ^= payload[i];
        }

        return checksum;
    }

    Status SPEAR::buildMessage(const char payload[], char output[])
    {
        uint16_t idx = 0;

        output[idx++] = STX;
        output[idx++] = (strlen(payload) >> 8) & 0xFF;
        output[idx++] = strlen(payload) & 0xFF;
        
        for (size_t i = 0; i < strlen(payload); ++i)
        {
            output[idx++] = payload[i];
        }
        
        output[idx++] = calculateChecksum(payload);
        output[idx++] = ETX;
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::VersionEnquiryService()
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }
        
        Send("{\"c\":2}");
        
        const uint16_t timeout = 1500;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");

        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     xSemaphoreGive(xSemaphoreSPEAR);
        //     return ret;
        // }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::VERSION_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::VERSION_ENQUIRY));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();

        const char* semanticVersion = obj["1"].as<const char*>();
        const uint32_t versionCode = obj["2"].as<uint32_t>();

        FW_VERSION_MEGA2560.SetSemanticVersion(semanticVersion);
        FW_VERSION_MEGA2560.SetVersionCode(versionCode);

        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::MemoryEnquiryService()
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        Send("{\"c\":3}");
        
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");

        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     xSemaphoreGive(xSemaphoreSPEAR);
        //     return ret;
        // }

        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::MEMORY_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::MEMORY_ENQUIRY));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();
        const uint32_t remained    = obj["1"].as<uint32_t>();
        const uint32_t usedStack   = obj["2"].as<uint32_t>();
        const uint32_t usedHeap    = obj["3"].as<uint32_t>();
        const uint32_t usedBSS     = obj["4"].as<uint32_t>();

        LOG_INFO(logger, "Remained %u, usedStack %u, usedHeap %u, usedBSS: %u",
            remained, usedStack, usedHeap, usedBSS);

        LOG_INFO(logger, "Initialized successfully");
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }
    
    Status SPEAR::StatusEnquiryService()
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }
        
        Send("{\"c\":4}");
        
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");

        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     xSemaphoreGive(xSemaphoreSPEAR);
        //     return ret;
        // }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::STATUS_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::STATUS_ENQUIRY));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        const uint32_t statusCode = doc["r"].as<uint32_t>();
        const Status::Code response = static_cast<Status::Code>(statusCode);
        Status statusMega = Status(response);
        LOG_INFO(logger, "statusCode %u", statusMega.c_str());
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::SignOnService()
    {
        const uint8_t MAX_TRIAL_COUNT = 5;
        uint8_t trialCount = 0;
        while (trialCount < MAX_TRIAL_COUNT)
        {
            if (receiveSignOn() == Status::Code::GOOD)
            {
                break;
            }
            ++trialCount;
        }

        if (trialCount == MAX_TRIAL_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO SIGN-ON FROM THE MEGA2560");
            return Status(Status::Code::BAD);
        }
        else
        {
            LOG_INFO(logger, "Sign-on from the MEGA2560");
            return Status(Status::Code::GOOD);
        }
    }

    Status SPEAR::SetJarvisProtocolConfig(const std::set<jvs::prt_e> link)
    {
        JsonDocument JarvisJson;
        JarvisJson["c"] = static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP);
        JsonObject body = JarvisJson["b"].to<JsonObject>();
        body["1"] = static_cast<uint8_t>(cfg_type_e::PROTOCOL);
        body["2"] = static_cast<uint8_t>(cfg_key_e::MODBUS);
        JsonObject config =  body["3"].to<JsonObject>();
        JsonArray configArray = config["l"].to<JsonArray>();
        for(auto& val : link)
        {
           uint8_t value = val == jvs::prt_e::PORT_2 ? 1 : 2;
           configArray.add(value);
        }

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);
        serializeJson(JarvisJson, payload, size);
        
        // writeJson(payload, "/spear/protocol/config.json");
        Send(payload);
        delay(80);
        return validateSetService();
    }

    Status SPEAR::SetJarvisLinkConfig(jvs::config::Base* cin, const jvs::cfg_key_e type)
    {
        Status ret = Status(Status::Code::UNCERTAIN);

        switch (type)
        {
        case jvs::cfg_key_e::RS232:
            // ret = setJarvisRs232Config();
            break;
        case jvs::cfg_key_e::RS485:
            ret = setJarvisRs485Config(cin);
            break;
        default:
            break;
        }

        return ret;
    }

    Status SPEAR::setJarvisRs485Config(jvs::config::Base* cin)
    {
        JsonDocument JarvisJson;

        JarvisJson["c"] = static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP);
        JsonObject body = JarvisJson["b"].to<JsonObject>();

        jvs::config::Rs485* data = Convert.ToRS485CIN(cin);
        jvs::prt_e port = data->GetPortIndex().second;
        if (port == jvs::prt_e::PORT_2)
        {
            body["1"] = static_cast<uint8_t>(cfg_type_e::LINK1);
            body["2"] = static_cast<uint8_t>(cfg_key_e::RS485);
            JsonObject config =  body["3"].to<JsonObject>();
            config["b"]  = static_cast<uint32_t>(data->GetBaudRate().second);
            config["d"] = static_cast<uint8_t>(data->GetDataBit().second);
            config["p"] = static_cast<uint8_t>(data->GetParityBit().second); 
            config["s"] = static_cast<uint8_t>(data->GetStopBit().second); 
        }
        else
        {
            body["1"] = static_cast<uint8_t>(cfg_type_e::LINK2);
            body["2"] = static_cast<uint8_t>(cfg_key_e::RS485);
            JsonObject config =  body["3"].to<JsonObject>();
            config["b"]  = static_cast<uint32_t>(data->GetBaudRate().second);
            config["d"] = static_cast<uint8_t>(data->GetDataBit().second);
            config["p"] = static_cast<uint8_t>(data->GetParityBit().second); 
            config["s"] = static_cast<uint8_t>(data->GetStopBit().second);  
        }

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        // std::string path = port == jvs::prt_e::PORT_2 ? "/spear/link1/config.json" : "/spear/link2/config.json";
        // writeJson(payload, path);

        LOG_DEBUG(logger,"payload : %s",payload);
        Send(payload);
        delay(100);
        return validateSetService();
    }

    Status SPEAR::receiveSignOn()
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const size_t timeout = 2000;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        
        Send("{\"c\":1,\"s\":0}");
        LOG_VERBOSE(logger, "Initialized successfully");
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }

    void SPEAR::writeJson(const char payload[], const std::string& path)
    {
        File file = esp32FS.Open(path, "w", true);
        for (size_t i = 0; i < strlen(payload); ++i)
        {
            file.write(payload[i]);
        }
        file.close();
    }

    Status SPEAR::readJson(const std::string& path, const uint8_t size, char payload[])
    {
        File file = esp32FS.Open(path);

        if (!file)
        {
            LOG_ERROR(logger,"FAILED TO OPEN FILE");
            return Status(Status::Code::BAD);
        }

        if (file.size() == 0)
        {
            LOG_INFO(logger,"File is empty");
            file.close();
            return Status(Status::Code::GOOD_NO_DATA);
        }
        ASSERT((file.size() < size), "FILE SIZE OVERFLOW");

        uint8_t idx = 0;
        while (file.available() > 0)
        {
            payload[idx++] = file.read();
        }
        file.close();
        return Status(Status::Code::GOOD);
    }

    // Status SPEAR::resendSetService()
    // {
    //     const uint8_t MAX_TRIAL_COUNT = 5;
    //     uint8_t trialCount = 0;
    //     while (trialCount < MAX_TRIAL_COUNT)
    //     {
    //         if (resendSetConfig("/spear/link1/config.json") == Status(Status::Code::GOOD))
    //         {
    //             trialCount = 0;
    //             break;
    //         }
    //         ++trialCount;
    //     }

    //     if (trialCount == MAX_TRIAL_COUNT)
    //     {
    //         LOG_ERROR(logger, "FAILED TO RESEND SETSERVICE : LINK1");
    //         return Status(Status::Code::BAD);
    //     }
   
    //     while (trialCount < MAX_TRIAL_COUNT)
    //     {
    //         if (resendSetConfig("/spear/link2/config.json") == Status(Status::Code::GOOD))
    //         {
    //             trialCount = 0;
    //             break;
    //         }
    //         ++trialCount;
    //     }

    //     if (trialCount == MAX_TRIAL_COUNT)
    //     {
    //         LOG_ERROR(logger, "FAILED TO RESEND SETSERVICE : LINK2");
    //         return Status(Status::Code::BAD);
    //     }

    //     while (trialCount < MAX_TRIAL_COUNT)
    //     {
    //         if (resendSetConfig("/spear/protocol/config.json") == Status(Status::Code::GOOD))
    //         {
    //             trialCount = 0;
    //             break;
    //         }
    //         ++trialCount;
    //     }

    //     if (trialCount == MAX_TRIAL_COUNT)
    //     {
    //         LOG_ERROR(logger, "FAILED TO RESEND SETSERVICE : PROTOCOL");
    //         return Status(Status::Code::BAD);
    //     }
        
    //     return Status(Status::Code::GOOD);
    // }

    // Status SPEAR::resendSetConfig(const std::string& path)
    // {
    //     const uint8_t size = 64;
    //     char payload[64] = {'\0'};
        
    //     Status ret = readJson(path, size, payload);
    //     if (ret != Status(Status::Code::GOOD))
    //     {
    //         return ret;
    //     }

    //     if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
    //     {
    //         LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
    //         return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
    //     }

    //     LOG_INFO(logger,"SEND MSG : %s",payload);

    //     Send(payload);
    //     delay(80);
    //     return validateSetService();
    // }

    Status SPEAR::validateSetService()
    {
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");
            
        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     xSemaphoreGive(xSemaphoreSPEAR);
        //     return ret;
        // }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        const uint32_t statusCode = doc["r"].as<uint32_t>();
        const Status::Code response = static_cast<Status::Code>(statusCode);
        LOG_INFO(logger, "statusCode: %s", Status(response).c_str());
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(response);
    }

    Status SPEAR::PollService(spear_daq_msg_t* daq)
    {
        JsonDocument JarvisJson;
        
        JarvisJson["c"] = static_cast<uint8_t>(spear_cmd_e::DAQ_POLL);
        JsonObject body = JarvisJson["b"].to<JsonObject>();
        body["1"] = daq->Link == jvs::prt_e::PORT_2 ? static_cast<uint8_t>(cfg_type_e::LINK1) : static_cast<uint8_t>(cfg_type_e::LINK2);
        body["2"] = daq->SlaveID;
        body["3"] = static_cast<uint8_t>(daq->Area);
        body["4"] = daq->Address;
        body["5"] = daq->Quantity;

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const uint8_t payloadSize = measureJson(JarvisJson) + 1;
        char payload[payloadSize];
        memset(payload, 0, payloadSize);
        serializeJson(JarvisJson, payload, payloadSize);

        while (Serial2.available())
        {
            Serial2.read();
            delay(1);
        }
        
        Send(payload);

        const uint16_t timeout = 3000;
        const uint16_t size = 1024;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");
            
        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     return ret;
        // }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::DAQ_POLL))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::DAQ_POLL));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        JsonObject response = doc["r"].as<JsonObject>();
        uint16_t modbusResult = response["6"].as<uint16_t>();
        if (modbusResult != static_cast<uint16_t>(mb_status_e::SUCCESS))
        {
            LOG_ERROR(logger, "BAD MODBUS STATUS CODE: 0x%02X", modbusResult);
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        bool isValid = true;
        isValid &= daq->Link == static_cast<jvs::prt_e>(response["1"].as<uint8_t>());
        isValid &= daq->SlaveID == response["2"].as<uint8_t>();
        isValid &= daq->Area == static_cast<jvs::node_area_e>(response["3"].as<uint16_t>());
        isValid &= daq->Address == response["4"].as<uint16_t>();
        isValid &= daq->Quantity == response["5"].as<uint16_t>();

        if (isValid == false)
        {
            while (Serial2.available())
            {
                Serial2.read();
                delay(1);
            }
            LOG_ERROR(logger,"INVALID DATA");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
        
        JsonArray value = response["7"].as<JsonArray>();
        for (auto val : value)
        {
            daq->PolledValuesVector.emplace_back(val.as<uint16_t>());
        }
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::ExecuteService(spear_remote_control_msg_t msg)
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }
        
        char command[64] = {'\0'};
        sprintf(command, "{\"c\":49,\"b\":{\"1\":%u,\"2\":%u,\"3\":%u,\"4\":%u,\"5\":%u}}", 
            static_cast<uint8_t>(msg.Link), msg.SlaveID, static_cast<uint8_t>(msg.Area), msg.Address, msg.Value);
        Send(command);
        
        const uint16_t timeout = 2000;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            LOG_WARNING(logger,"ERROR! , ret : %s",ret.c_str());
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }

        // if (doc["c"].as<uint8_t>() == static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        // {
        //     LOG_INFO(logger,"Sign on request service !");
        //     Send("{\"c\":1,\"s\":0}");

        //     xSemaphoreGive(xSemaphoreSPEAR);

        //     ret = resendSetService();
        //     if (ret != Status::Code::GOOD)
        //     {
        //         LOG_ERROR(logger,"RESEND SETSERVICE ERROR! CORD : %s", ret.c_str());
        //     }
        //     xSemaphoreGive(xSemaphoreSPEAR);
        //     return ret;
        // }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::REMOTE_CONTROL))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::REMOTE_CONTROL));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        
        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }

        const uint32_t modbusState = doc["r"].as<uint32_t>();
        const mb_status_e response = static_cast<mb_status_e>(modbusState);
        if (response != mb_status_e::SUCCESS)
        {
            LOG_ERROR(logger, "RSC: %u", static_cast<uint32_t>(response));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD);
        }
        else
        {
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::GOOD);
        }
    }


    SPEAR spear;
}

#endif