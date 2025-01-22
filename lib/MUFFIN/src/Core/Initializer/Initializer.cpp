/**
 * @file Initializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크의 초기화를 담당하는 클래스를 정의합니다.
 * 
 * @date 2025-01-21
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <esp_system.h>
#include <Preferences.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "Core/Core.h"
#include "Core/Initializer/Initializer.h"
#include "Core/Include/Helper.h"
#include "Core/Task/JarvisTask.h"
#include "Core/Task/NetworkTask.h"
#include "DataFormat/JSON/JSON.h"

#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"

#include "JARVIS/Config/Network/CatM1.h"
#if defined(MODLINK_T2) || defined(MODLINK_B)
    #include "JARVIS/Config/Network/Ethernet.h"
#elif defined(MODLINK_B)
    #include "JARVIS/Config/Network/WiFi4.h"
#endif
#include "JARVIS/JARVIS.h"

#include "Network/CatM1/CatM1.h"
#if defined(MODLINK_T2) || defined(MODLINK_B)
    #include "Network/Ethernet/Ethernet.h"
#elif defined(MODLINK_B)
    #include "Network/WiFi4/WiFi4.h"
#endif

#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/SPEAR/SPEAR.h"

#include "Storage/ESP32FS/ESP32FS.h"




namespace muffin {

    void Initializer::StartOrCrash()
    {
        LOG_INFO(logger, "Ethernet MAC: %s", macAddress.GetEthernet());

        Status ret = esp32FS.Begin(false);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO MOUNT ESP32 FILE SYSTEM");
            vTaskDelay(UINT32_MAX / portTICK_PERIOD_MS);
            esp_restart();
        }
        LOG_INFO(logger, "Initialized ESP32 file system");
        
        ret = processResetReason();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO PROCESS RESET REASON");
            vTaskDelay(UINT32_MAX / portTICK_PERIOD_MS);
            esp_restart();
        }

        LOG_INFO(logger, "Initializer has started");
    }

    Status Initializer::processResetReason()
    {
        Preferences pf;
        const char* key = "prc";    // prc is short for "process reset reason"

        if (pf.begin("Initializer", false) == false)
        {
            LOG_ERROR(logger, "FAILED TO BEGIN NVS PARTITION");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        if (pf.isKey(key) == false)
        {
            pf.putChar(key, 0);
            if (pf.getChar(key, -1) != 0)
            {
                LOG_ERROR(logger, "FAILED TO WRITE PANIC RESET COUNT");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            LOG_INFO(logger, "Created a panic reset count");
        }

        int8_t panicResetCount = pf.getChar(key, -1);
        if (panicResetCount == -1)
        {
            LOG_ERROR(logger, "FAILED TO READ PANIC RESET COUNT");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        if (panicResetCount > MAX_RETRY_COUNT)
        {
            esp32FS.Remove(JARVIS_PATH);
            /*
             *  "/spear/protocol/config.json"
             *  "/spear/link1/config.json"
             *  "/spear/link2/config.json"
            */
        }
        
        const esp_reset_reason_t resetReason = esp_reset_reason();
        deviceStatus.SetResetReason(resetReason);
        LogResetReason(resetReason);

        if (resetReason != ESP_RST_PANIC)
        {
            return Status(Status::Code::GOOD);
        }
        
        pf.putChar(key, ++panicResetCount);
        if (panicResetCount != pf.getChar(key, -1))
        {
            LOG_ERROR(logger, "FAILED TO WRITE PANIC RESET COUNT");
            pf.remove(key);
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        return Status(Status::Code::GOOD);
    }

    Status Initializer::Configure(const bool hasJARVIS, const bool hasOTA)
    {
        if (hasJARVIS || hasOTA)
        {
            mHasJARVIS = hasJARVIS;
            mHasUpdate = hasOTA;
            ASSERT((false), "UNIMPLEMENTED SERVICE");
            // return configureWithoutJarvis();
        }

        Status ret = esp32FS.DoesExist(JARVIS_PATH);

        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            LOG_INFO(logger, "Start to create default config");
            ret = createDefaultConfig();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CREATE DEFAULT CONFIG");
                return ret;
            }
            LOG_INFO(logger, "Created default JARVIS config");
        }

        for (uint8_t count = 0; count < MAX_RETRY_COUNT; ++count)
        {
            ret = implementConfigure();
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Configured successfully");
                return ret;
            }
            
            LOG_WARNING(logger, "[TRIAL: #%u] CONFIGURATION WAS UNSUCCESSFUL: %s", count, ret.c_str());
            vTaskDelay((1 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
        }

        LOG_ERROR(logger, "FAILED TO CONFIGURE JARVIS");
        return ret;
    }

    Status Initializer::createDefaultConfig()
    {
        File file = esp32FS.Open(JARVIS_PATH, "w", true);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN JARVIS DIRECTORY");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        LOG_DEBUG(logger, "Opened JARVIS file with 'WRITE' mode");

        JsonDocument doc;
        doc["ver"] = "v1";

        JsonObject cnt = doc["cnt"].to<JsonObject>();
        cnt["rs232"].to<JsonArray>();
        cnt["rs485"].to<JsonArray>();
        cnt["wifi"].to<JsonArray>();
        cnt["eth"].to<JsonArray>();
        cnt["mbrtu"].to<JsonArray>();
        cnt["mbtcp"].to<JsonArray>();
        cnt["node"].to<JsonArray>();
        cnt["alarm"].to<JsonArray>();
        cnt["optime"].to<JsonArray>();
        cnt["prod"].to<JsonArray>();

        JsonArray catm1 = cnt["catm1"].to<JsonArray>();
        JsonObject _catm1 = catm1.add<JsonObject>();
        _catm1["md"]    = "LM5";
        _catm1["ctry"]  = "KR";

        JsonArray op    = cnt["op"].to<JsonArray>();
        JsonObject _op  = op.add<JsonObject>();
        _op["snic"]      = "lte";
        _op["exp"]       = true;
        _op["intvPoll"]  = 1;
        _op["intvSrv"]   = 60;
        _op["rst"]       = false;

        const size_t size = measureJson(doc) + 1;
        char buffer[size] = {'\0'};
        serializeJson(doc, buffer, size);
        doc.clear();

        // Status ret = ConnectToBrokerEthernet();
        // StartEthernetTask();
        // if (mHasJARVIS)
        // {
        //     core.StartJarvisTask();
        // }
        // return ret;

        file.write(reinterpret_cast<uint8_t*>(buffer), size);
        file.flush();
        file.close();
        ASSERT((file == false), "FILE MUST BE FLUSHED AND CLOSED AFTER WRITING");

        file = esp32FS.Open(JARVIS_PATH);
        if (size != file.size())
        {
            LOG_ERROR(logger, "FAILED TO WRITE: size != file.size()");
            file.close();

            for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
            {
                if (esp32FS.Remove(JARVIS_PATH) == Status::Code::GOOD)
                {
                    break;
                }
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        LOG_INFO(logger, "Default JARVIS config has been saved");
        return Status(Status::Code::GOOD);
    }

    Status Initializer::implementConfigure()
    {
        JSON json;
        JsonDocument doc;
        Status ret(Status::Code::UNCERTAIN);
        {
            File file = esp32FS.Open(JARVIS_PATH, "r", false);
            const size_t size = file.size();
            char buffer[size + 1] = {'\0'};
            for (size_t idx = 0; idx < size; ++idx)
            {
                buffer[idx] = file.read();
            }
            file.close();
            ASSERT((file == false), "FILE MUST BE CLOSED");
            LOG_DEBUG(logger, "JARVIS: %s", buffer);

            ret = json.Deserialize(buffer, &doc);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DESERIALIZE: %s", ret.c_str());
                return ret;
            }
            LOG_INFO(logger, "Deserialized JARVIS config file");
        }
        
    #if defined(DEBUG)
        doc["cnt"].remove("catm1");
        doc["cnt"]["catm1"].to<JsonArray>();
        JsonObject eth = doc["cnt"]["eth"].add<JsonObject>();
        eth["dhcp"]  = true;
        eth["ip"]    = NULL;
        eth["snm"]   = NULL;
        eth["gtw"]   = NULL;
        eth["dns1"]  = NULL;
        eth["dns2"]  = NULL;
        JsonObject op = doc["cnt"]["op"][0].as<JsonObject>();
        op["snic"] = "eth";
    #else
        std::abort();
    #endif

        ASSERT((jarvis == nullptr), "The instance <JARVIS* jarvis> MUST BE NULL");
        jarvis = new(std::nothrow) JARVIS();
        jarvis->Validate(doc);
        doc.clear();
        
        ApplyJarvisTask();
        return Status(Status::Code::GOOD);
    }
}