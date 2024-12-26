/**
 * @file SPEAR.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPEAR 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-12-24
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/FirmwareVersion/FirmwareVersion.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "SPEAR.h"


namespace muffin {

    Status SPEAR::Init()
    {
        Serial2.begin(115200);
        return SignOnService();
    }

    Status SPEAR::Reset()
    {
        pinMode(RESET_PIN, OUTPUT);
        digitalWrite(RESET_PIN, HIGH);
        return SignOnService();
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
            if (Serial2.available() < (HEAD_SIZE + TAIL_SIZE))
            {
                const uint32_t elapsedMillis = millis() - startedMillis;
                const uint32_t remainedMillis = timeoutMillis - elapsedMillis;
                delay(remainedMillis > 50 ? 50 : remainedMillis);
                continue;
            }

            while (Serial2.available())
            {
                int rxd = Serial2.read();
                if (rxd == -1)
                {
                    continue;
                }

                payload[length++] = rxd;
                if (payload[length - 1] == 0x03)
                {
                    goto RECEIVED_ETX;
                }
            }
        }

        if (length == 0)
        {
            return Status(Status::Code::BAD_TIMEOUT);
        }
        else
        {
            return Status(Status::Code::BAD_DATA_LOST);
        }

    RECEIVED_ETX:
        if (payload[0] != STX || payload[length - 1] != ETX)
        {
            return Status(Status::Code::BAD_DATA_LOST);
        }
        
        const uint16_t calculatedLength = length - HEAD_SIZE - TAIL_SIZE;
        const uint16_t receivedLength   = ((uint16_t)payload[1] << 8) | payload[2];
        if (calculatedLength != receivedLength)
        {
            return Status(Status::Code::BAD_DATA_LOST);
        }
        
        const uint8_t receivedChecksum   = payload[length - 2];
        length = receivedLength;
        
        for (uint16_t i = 0; i < length; ++i)
        {
            payload[i] = payload[HEAD_SIZE + i];
        }
        payload[length] = '\0';

        const uint8_t calculatedChecksum = calculateChecksum(payload);
        if (calculatedChecksum != receivedChecksum)
        {
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
        Send("{\"c\":2}");
        
        const uint16_t timeout = 1000;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        LOG_DEBUG(logger, "Version: %s, %s", ret.c_str(), buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::VERSION_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::VERSION_ENQUIRY));
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();

        const char* semanticVersion = obj["1"].as<const char*>();
        const uint32_t versionCode = obj["2"].as<uint32_t>();
        
        LOG_INFO(logger, "Semantic Version: %s", semanticVersion);
        LOG_INFO(logger, "vc: %U", versionCode);

        FW_VERSION_MEGA2560.SetSemanticVersion(semanticVersion);
        FW_VERSION_MEGA2560.SetVersionCode(versionCode);
        
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::MemoryEnquiryService()
    {
        Send("{\"c\":3}");
        
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::MEMORY_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::MEMORY_ENQUIRY));
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
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
        return Status(Status::Code::GOOD);
    }
    
    Status SPEAR::StatusEnquiryService()
    {
        Send("{\"c\":4}");
        
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::STATUS_ENQUIRY))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::STATUS_ENQUIRY));
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        const uint32_t statusCode = doc["r"].as<uint32_t>();
        const Status::Code response = static_cast<Status::Code>(statusCode);
        Status statusMega = Status(response);
        LOG_INFO(logger, "statusCode %u", statusMega.c_str());
        
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

    Status SPEAR::SetJarvisProtocolConfig(const std::set<jarvis::prt_e> link)
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
           uint8_t value = val == jarvis::prt_e::PORT_2 ? 1 : 2;
           configArray.add(value);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        
        writeJson(payload, "/spear/protocol/config.json");
        Send(payload);
        return validateSetService();
    }

    Status SPEAR::SetJarvisLinkConfig(jarvis::config::Base* cin, const jarvis::cfg_key_e type)
    {
        switch (type)
        {
        case jarvis::cfg_key_e::RS232:
            // setJarvisRs232Config();
            break;
        case jarvis::cfg_key_e::RS485:
            setJarvisRs485Config(cin);
            break;
        default:
            break;
        }

        return Status(Status::Code::GOOD);
    }

    Status SPEAR::setJarvisRs485Config(jarvis::config::Base* cin)
    {
        JsonDocument JarvisJson;

        JarvisJson["c"] = static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP);
        JsonObject body = JarvisJson["b"].to<JsonObject>();

        jarvis::config::Rs485* data = Convert.ToRS485CIN(cin);
        jarvis::prt_e port = data->GetPortIndex().second;
        if (port == jarvis::prt_e::PORT_2)
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

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        std::string path = port == jarvis::prt_e::PORT_2 ? "/spear/link1/config.json" : "/spear/link2/config.json";
        writeJson(payload, path);

        Send(payload);
        return validateSetService();
    }

    Status SPEAR::receiveSignOn()
    {
        const size_t timeout = 2000;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }

        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST));
            return Status(Status::Code::BAD);
        }
        
        Send("{\"c\":1,\"s\":0}");
        LOG_INFO(logger, "Initialized successfully");
        return Status(Status::Code::GOOD);
    }

    void SPEAR::writeJson(const char payload[], const std::string& path)
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        File file = esp32FS.Open(path, "w", true);

        for (size_t i = 0; i < strlen(payload); ++i)
        {
            file.write(payload[i]);
        }
        file.close();
    }

    Status SPEAR::readJson(const std::string& path, const uint8_t size, char payload[])
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
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

    Status SPEAR::resendSetService()
    {
        Status ret = resendSetConfig("/spear/protocol/config.json");
        if (ret != Status(Status::Code::GOOD))
        {
           return ret;
        }

        ret = resendSetConfig("/spear/link1/config.json");
        if (ret != Status(Status::Code::GOOD))
        {
           return ret;
        }

        ret = resendSetConfig("/spear/link2/config.json");
        if (ret != Status(Status::Code::GOOD))
        {
           return ret;
        }
        
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::resendSetConfig(const std::string& path)
    {
        const uint8_t size = 64;
        char payload[64] = {'\0'};
        
        Status ret = readJson(path, size, payload);
        if (ret != Status(Status::Code::GOOD))
        {
            return ret;
        }

        Send(payload);
        return validateSetService();
    }

    Status SPEAR::validateSetService()
    {
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        LOG_DEBUG(logger, "SPEAR RxD: %s", buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP));
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        const uint32_t statusCode = doc["r"].as<uint32_t>();
        const Status::Code response = static_cast<Status::Code>(statusCode);
        LOG_INFO(logger, "statusCode: %s", Status(response).c_str());
        return Status(response);
    }

    Status SPEAR::PollService(spear_daq_msg_t* daq)
    {
        JsonDocument JarvisJson;
        
        JarvisJson["c"] = static_cast<uint8_t>(spear_cmd_e::DAQ_POLL);
        JsonObject body = JarvisJson["b"].to<JsonObject>();
        body["1"] = daq->Link == jarvis::prt_e::PORT_2 ? static_cast<uint8_t>(cfg_type_e::LINK1) : static_cast<uint8_t>(cfg_type_e::LINK2);
        body["2"] = daq->SlaveID;
        body["3"] = static_cast<uint8_t>(daq->Area);
        body["4"] = daq->Address;
        body["5"] = daq->Quantity;

        const uint8_t payloadSize = measureJson(JarvisJson) + 1;
        char payload[payloadSize];
        memset(payload, 0, payloadSize);

        serializeJson(JarvisJson, payload, payloadSize);
        LOG_INFO(logger,"SEND MSG : %s",payload);

        Send(payload);

        const uint16_t timeout = 1000;
        const uint16_t size = 1024;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        LOG_INFO(logger,"Receive MSG : %s",buffer);
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::DAQ_POLL))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::DAQ_POLL));
            return Status(Status::Code::BAD);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }
        JsonObject response = doc["r"].as<JsonObject>();
        uint16_t modbusResult = response["6"].as<uint16_t>();
        if (modbusResult != static_cast<uint16_t>(mb_status_e::SUCCESS))
        {
            LOG_ERROR(logger, "BAD MODBUS STATUS CODE: 0x%02X", modbusResult);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        JsonArray value = response["7"].as<JsonArray>();
        for (auto val : value)
        {
            daq->PolledValuesVector.emplace_back(val.as<uint16_t>());
        }
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::ExecuteService(spear_remote_control_msg_t msg)
    {
        char command[64] = {'\0'};
        sprintf(command, "{\"c\":49,\"b\":{\"1\":%u,\"2\":%u,\"3\":%u,\"4\":%u,\"5\":%u}}", 
            static_cast<uint8_t>(msg.Link), msg.SlaveID, static_cast<uint8_t>(msg.Area), msg.Address, msg.Value);
        Send(command);
        
        const uint8_t timeout = 100;
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        Status ret = Receive(timeout, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        JSON json;
        JsonDocument doc;
        ret = json.Deserialize(buffer, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE");
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::REMOTE_CONTROL))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::REMOTE_CONTROL));
            return Status(Status::Code::BAD);
        }
        
        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        const uint32_t modbusState = doc["r"].as<uint32_t>();
        const mb_status_e response = static_cast<mb_status_e>(modbusState);
        if (response != mb_status_e::SUCCESS)
        {
            LOG_ERROR(logger, "RSC: %u", static_cast<uint32_t>(response));
            return Status(Status::Code::BAD);
        }
        else
        {
            LOG_INFO(logger, "Modbus State: %u", modbusState);
            return Status(Status::Code::GOOD);
        }
    }


    SPEAR spear;
}