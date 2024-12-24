/**
 * @file SPEAR.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2024-12-11
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "DataFormat/JSON/JSON.h"
#include "Core/Task/UpdateTask.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
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
    
        LOG_INFO(logger, "Sign-on from the MEGA2560");
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::reset()
    {
        pinMode(RESET_PIN, OUTPUT);
        digitalWrite(RESET_PIN, HIGH);
        
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
            LOG_ERROR(logger, "FAILED TO RESET FROM THE MEGA2560");
            return Status(Status::Code::BAD);
        }
    
        LOG_INFO(logger, "Reset from the MEGA2560");
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::VersionEnquiryService()
    {
        req_head_t head;
        JSON json;
        
        head.Code  = static_cast<uint8_t>(spear_cmd_e::VERSION_ENQUIRY);
        const uint8_t size = 32;
        char payload[size];
        memset(payload, 0, size);
        json.Serialize(head, size, payload);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }
        Receive();

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);
        
        JsonDocument doc;
        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != head.Code)
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), head.Code);
            return Status(Status::Code::BAD);
        }
        

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();

        FIRMWARE_VERSION_MCU2 = obj["semanticVersion"].as<std::string>();
        FIRMWARE_VERSION_CODE_MCU2 = obj["versionCode"].as<uint32_t>();

        LOG_INFO(logger, "Initialized successfully");
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::MemoryEnquiryService()
    {
        req_head_t head;
        JSON json;
        
        head.Code  = static_cast<uint8_t>(spear_cmd_e::MEMORY_ENQUIRY);
        head.Index = mSequenceID++;

        const uint8_t size = 32;
        char payload[size];
        memset(payload, 0, size);
        json.Serialize(head, size, payload);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }
        Receive();

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);
        JsonDocument doc;
        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != head.Code)
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), head.Code);
            return Status(Status::Code::BAD);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();

        const uint32_t remained = obj["remained"].as<uint32_t>();
        const uint32_t usedStack = obj["usedStack"].as<uint32_t>();
        const uint32_t usedHeap = obj["usedHeap"].as<uint32_t>();
        LOG_INFO(logger, "remained %lu byte , usedStack %lu byte , usedHeap %lu byte ",remained,usedStack,usedHeap);

        LOG_INFO(logger, "Initialized successfully");
        return Status(Status::Code::GOOD);
    }
    

    Status SPEAR::StatusEnquiryService()
    {
        req_head_t head;
        JSON json;
        
        head.Code  = static_cast<uint8_t>(spear_cmd_e::STATUS_ENQUIRY);
        head.Index = mSequenceID++;

        const uint8_t size = 32;
        char payload[size];
        memset(payload, 0, size);

        json.Serialize(head, size, payload);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);
        JsonDocument doc;
        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != head.Code)
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), head.Code);
            return Status(Status::Code::BAD);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();
        uint32_t statusCode = obj["statusCode"].as<uint32_t>();    
        Status::Code response = static_cast<Status::Code>(statusCode);
        Status statusMega = Status(response);
        LOG_INFO(logger, "statusCode %s",statusMega.c_str());
        
        LOG_INFO(logger, "Initialized successfully");
        return Status(Status::Code::GOOD);
    }

    Status SPEAR::ExecuteService(spear_remote_control_msg_t msg)
    {
        JSON json;
        
        msg.Head.Code  = static_cast<uint8_t>(spear_cmd_e::REMOTE_CONTROL);
        msg.Head.Index = mSequenceID++;

        const uint8_t size = 32;
        char payload[size];
        memset(payload, 0, size);
        json.Serialize(msg, size, payload);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);

        JsonDocument doc;
        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            return ret;
        }
        
        if (doc["c"].as<uint8_t>() != msg.Head.Code)
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), msg.Head.Code);
            return Status(Status::Code::BAD);
        }

        if (doc["index"].as<uint8_t>() != msg.Head.Index)
        {
            LOG_ERROR(logger, "INVALID INDEX: 0x%02X != 0x%02X", 
                doc["index"].as<uint8_t>(), msg.Head.Index);
            return Status(Status::Code::BAD);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD);
        }

        JsonObject obj = doc["r"].as<JsonObject>();
        uint32_t statusCode = obj["result"].as<uint32_t>();    
        Status::Code response = static_cast<Status::Code>(statusCode);
        Status statusMega = Status(response);
        LOG_INFO(logger, "statusCode %s",statusMega.c_str());
        
        LOG_INFO(logger, "Initialized successfully");
        return Status(Status::Code::GOOD);
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

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        
        writeJson(payload, "/spear/protocol/config.json");

        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        return validateSetService();
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

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        LOG_INFO(logger,"SEND MSG : %s",payload);

        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }
        

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);
        JsonDocument doc;
        JSON json;

        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        if (doc["c"].as<uint8_t>() != JarvisJson["c"].as<uint8_t>())
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), JarvisJson["c"].as<uint8_t>());
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        JsonObject response = doc["r"].to<JsonObject>();
        uint16_t modbusResult = response["6"].as<uint16_t>();
        if (modbusResult != static_cast<uint16_t>(mb_status_e::SUCCESS))
        {
            LOG_ERROR(logger, "BAD MODBUS STATUS CODE: 0x%02X", modbusResult);
        }
        JsonArray value = response["7"].as<JsonArray>();
        for (auto val : value)
        {
            daq->PolledValuesVector.emplace_back(val.as<uint16_t>());
        }
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
        
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

        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        const uint8_t size = measureJson(JarvisJson) + 1;
        char payload[size];
        memset(payload, 0, size);

        serializeJson(JarvisJson, payload, size);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        std::string path = port == jarvis::prt_e::PORT_2 ? "/spear/link1/config.json" : "/spear/link2/config.json";
        writeJson(payload, path);

        buildMessage(payload);
        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_TIMEOUT);
        }

        return validateSetService();
    }

    Status SPEAR::receiveSignOn()
    {
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        resp_head_t head;
        JSON json;
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE MEGA2560");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_TIMEOUT);
        }

        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);

        JsonDocument doc;
        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            xSemaphoreGive(xSemaphoreSPEAR);
            return ret;
        }
        

        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::SIGN_ON_REQUEST))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"], spear_cmd_e::SIGN_ON_REQUEST);
            head.Status = static_cast<uint32_t>(Status::Code::BAD);
        }

        mSequenceID = doc["index"].as<uint8_t>();
        head.Code = doc["c"].as<uint8_t>();
        head.Index = mSequenceID;
        head.Status = static_cast<uint32_t>(Status::Code::GOOD);

        const uint8_t size = 36;
        char payload[size];
        memset(payload, 0, size);
        json.Serialize(head, size, payload);
        LOG_INFO(logger,"SEND MSG : %s",payload);
        buildMessage(payload);
        send();

        LOG_INFO(logger, "Initialized successfully");
        xSemaphoreGive(xSemaphoreSPEAR);
        return Status(Status::Code::GOOD);
    }

    uint8_t SPEAR::Count() const
    {
        return mQueueRxD.size();
    }

    Status SPEAR::Peek(spear_msg_t* message)
    {
        ASSERT((message != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        // ASSERT((message->Frame != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        if (mQueueRxD.size() == 0)
        {
            LOG_ERROR(logger, "NO RECEIVED MESSAGE");
            return Status(Status::Code::BAD_NO_DATA);
        }

        spear_msg_t* retrieved = mQueueRxD.front();
        message->Length = retrieved->Length;
        message->Frame = (char*)malloc(message->Length);
        strncpy(message->Frame, mQueueRxD.front()->Frame, message->Length);

        return Status(Status::Code::GOOD);
    }

    Status SPEAR::Retrieve(spear_msg_t* message)
    {
        ASSERT((message != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        ASSERT((message->Frame != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        if (mQueueRxD.size() == 0)
        {
            LOG_ERROR(logger, "NO RECEIVED MESSAGE");
            return Status(Status::Code::BAD_NO_DATA);
        }

        spear_msg_t* retrieved = mQueueRxD.front();
        message->Length = retrieved->Length;
        message->Frame = (char*)malloc(message->Length);
        strncpy(message->Frame, mQueueRxD.front()->Frame, message->Length);
        
        mQueueRxD.pop();
        free(retrieved->Frame);
        free(retrieved);

        return Status(Status::Code::GOOD);
    }

    void SPEAR::Send(char payload[])
    {
        buildMessage(payload);
        send();
    }

    void SPEAR::Receive()
    {
        if (Serial2.available() < (HEAD_SIZE + TAIL_SIZE))
        {
            return;
        }
        const uint16_t BASE_SIZE = HEAD_SIZE + 64 + TAIL_SIZE;

        spear_msg_t* message = (spear_msg_t*)malloc(sizeof(spear_msg_t));
        if (message == nullptr)
        {
            LOG_ERROR(logger, "BAD OUT OF MEMROY");
            return;
        }
        uint16_t allocatedBytes = BASE_SIZE;
        
        message->Frame   = (char*)malloc(BASE_SIZE);
        message->Length  = 0;
        memset(message->Frame, '\0', BASE_SIZE);

        while (Serial2.available())
        {
            int rxd = Serial2.read();
            if (rxd == -1)
            {
                continue;
            }

            if (message->Length == allocatedBytes)
            {
                char* buffer = (char*)malloc(2 * allocatedBytes);
                if (buffer == nullptr)
                {
                    LOG_ERROR(logger, "BAD OUT OF MEMROY");
                    return;
                }
                
                for (uint16_t i = 0; i < allocatedBytes; i++)
                {
                    buffer[i] = message->Frame[i];
                }
                free(message->Frame);
                message->Frame = buffer;
                allocatedBytes *= 2;
                LOG_DEBUG(logger, "Resized the buffer: %u", allocatedBytes);
            }

            message->Frame[message->Length++] = rxd;
            if (message->Frame[message->Length - 1] == 0x03)
            {
                break;
            }
        }

        if (message->Frame[0] != STX || message->Frame[message->Length - 1] != ETX)
        {
            LOG_ERROR(logger, "INVALID STX OR ETX BYTE RECEIVED");
            return;
        }

        const uint16_t calculatedLength = message->Length - HEAD_SIZE - TAIL_SIZE;
        const uint16_t receivedLength   = ((uint16_t)message->Frame[1] << 8) | message->Frame[2];

        if (calculatedLength != receivedLength)
        {
            LOG_ERROR(logger, "INVALID PAYLOAD LENGTH: %u != %u", calculatedLength, receivedLength);
            return;
        }

        const uint8_t calculatedChecksum = calculateChecksum(*message);
        const uint8_t receivedChecksum   = message->Frame[message->Length - 2];
        if (calculatedChecksum != receivedChecksum)
        {
            LOG_ERROR(logger, "INVALID CHECKSUM: 0x%02X != 0x%02X", calculatedChecksum, receivedChecksum);
            return;
        }

        LOG_DEBUG(logger, "Calculated Checksum: 0x%02X", calculatedChecksum);
        LOG_DEBUG(logger, "Received   Checksum: 0x%02X", receivedChecksum);
        
        message->Length = receivedLength;
        for (uint16_t i = 0; i < message->Length; ++i)
        {
            message->Frame[i] = message->Frame[HEAD_SIZE + i];
        }
        message->Frame[message->Length + 1] = '\0';
        
        mQueueRxD.push(message);
        LOG_DEBUG(logger, "RxD Queue Size: %u", mQueueRxD.size());
    }

    uint8_t SPEAR::calculateChecksum(char payload[])
    {
        ASSERT((payload != nullptr), "INPUT PARAMETER CANNOT BE A NULL POINTER");

        const uint16_t length = strlen(payload);
        ASSERT((length > 0), "INPUT PARAMETER MUST NOT BE AN EMPTY PAYLOAD");

        uint8_t checksum = 0x00;
        const uint8_t lengthHIGH  = (length >> 8) & 0xFF;
        const uint8_t lengthLOW   = length & 0xFF;
        checksum ^= lengthHIGH;
        checksum ^= lengthLOW;

        for (uint16_t i = 0; i < length; ++i)
        {
            checksum ^= payload[i];
        }
        return checksum;
    }

    uint8_t SPEAR::calculateChecksum(const spear_msg_t& message)
    {
        ASSERT((message.Length > 0), "INPUT PARAMETER MUST NOT BE AN EMPTY PAYLOAD");

        const uint8_t lengthHIGH  = (message.Frame[1] >> 8) & 0xFF;
        const uint8_t lengthLOW   = message.Frame[2] & 0xFF;
        const uint16_t length = ((uint16_t)lengthHIGH << 8) | lengthLOW;
        
        uint8_t checksum = 0x00;
        checksum ^= lengthHIGH;
        checksum ^= lengthLOW;

        for (uint16_t i = HEAD_SIZE; i < HEAD_SIZE + length; ++i)
        {
            checksum ^= message.Frame[i];
        }
        return checksum;
    }

    void SPEAR::buildMessage(char payload[])
    {
        spear_msg_t* message = (spear_msg_t*)malloc(sizeof(spear_msg_t));
        message->Length = HEAD_SIZE + strlen(payload) + TAIL_SIZE;
        message->Frame  = (char*)malloc(message->Length);

        message->Frame[0] = STX;
        message->Frame[1] = (strlen(payload) >> 8) & 0xFF;
        message->Frame[2] = strlen(payload) & 0xFF;
        
        for (uint16_t idx = 0; idx < strlen(payload); ++idx)
        {
            message->Frame[idx + HEAD_SIZE] = payload[idx];
        }

        message->Frame[message->Length - 2] = calculateChecksum(payload);
        message->Frame[message->Length - 1] = ETX;

        mQueueTxD.push(message);
    }

    void SPEAR::send()
    {
        while (mQueueTxD.size())
        {
            LOG_DEBUG(logger, "TxD Queue Size: %u", mQueueTxD.size());
            spear_msg_t* message = mQueueTxD.front();
            LOG_DEBUG(logger, "Size: %u", message->Length);

            for (size_t i = 0; i < message->Length; i++)
            {
                Serial.print(message->Frame[i],HEX);
                Serial.print(",");
            }
            
            Serial.println();
            Serial2.write(message->Frame, message->Length);
            Serial2.flush();

            mQueueTxD.pop();
            free(message->Frame);
            free(message);
        }
    }

    void SPEAR::writeJson(const char payload[],const std::string& path)
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        File file = esp32FS.Open(path, "w", true);

        for (size_t i = 0; i < strlen(payload); ++i)
        {
            file.write(payload[i]);
        }

        file.close();
    }

    Status SPEAR::readJson(const std::string& path, std::string* payload)
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

        while (file.available() > 0)
        {
            payload += file.read();
        }
        file.close();

        LOG_INFO(logger,"JSON : %s",payload->c_str());
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
        std::string payloadStr;
        Status ret = readJson(path, &payloadStr);
        LOG_INFO(logger,"read json : %s",payloadStr.c_str());
        if (ret != Status(Status::Code::GOOD))
        {
            return ret;
        }
        char* payload = new char[payloadStr.size() + 1];
        strcpy(payload, payloadStr.c_str());
        
        if (xSemaphoreTake(xSemaphoreSPEAR, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "[SPEAR] THE READ MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        buildMessage(payload);

        send();
        const uint32_t timeout = millis();
        while (millis() - timeout < 1000)
        {
            Receive();
            if (mQueueRxD.size() > 0)
            {
                break;
            }
        }

        if (mQueueRxD.size() == 0)
        {
            LOG_WARNING(logger, "NO RESPONSE FROM THE ESP32");
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        xSemaphoreGive(xSemaphoreSPEAR);
        return validateSetService();
    }


    Status SPEAR::validateSetService()
    {
        spear_msg_t* message = mQueueRxD.front();
        mQueueRxD.pop();
        LOG_INFO(logger,"RECEIVE MSG : %s",message->Frame);
        JsonDocument doc;
        JSON json;

        Status ret = json.Deserialize(message->Frame, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE MESSAGE: %s", message->Frame);
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        if (doc["c"].as<uint8_t>() != static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP))
        {
            LOG_ERROR(logger, "INVALID CODE: 0x%02X != 0x%02X", 
                doc["c"].as<uint8_t>(), static_cast<uint8_t>(spear_cmd_e::JARVIS_SETUP));
            xSemaphoreGive(xSemaphoreSPEAR);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        if (doc["s"].as<uint32_t>() != static_cast<uint32_t>(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "BAD STATUS CODE: 0x%02X", doc["s"].as<uint32_t>());
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        uint32_t statusCode = doc["r"].as<uint32_t>();
        Status::Code response = static_cast<Status::Code>(statusCode);
        Status statusMega = Status(response);
        LOG_INFO(logger, "statusCode %s",statusMega.c_str());
        
        LOG_INFO(logger, "Initialized successfully");

        return Status(Status::Code::GOOD);
    }
}