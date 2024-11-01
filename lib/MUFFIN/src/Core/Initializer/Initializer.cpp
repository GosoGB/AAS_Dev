/**
 * @file Initializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크의 초기화를 담당하는 클래스를 정의합니다.
 * 
 * @date 2024-10-30
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "Core/Initializer/Initializer.h"
#include "Core/Include/Helper.h"
#include "Core/Task/JarvisTask.h"
#include "Core/Task/NetworkTask.h"
#include "DataFormat/JSON/JSON.h"

#include "IM/MacAddress/MacAddress.h"

#include "Jarvis/Config/Network/CatM1.h"
#if defined(MODLINK_T2) || defined(MODLINK_B)
    #include "Jarvis/Config/Network/Ethernet.h"
#elif defined(MODLINK_B)
    #include "Jarvis/Config/Network/WiFi4.h"
#endif
#include "Jarvis/Jarvis.h"

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

#include "Storage/ESP32FS/ESP32FS.h"




namespace muffin {
    
    Initializer::Initializer()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Initializer::~Initializer()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    void Initializer::StartOrCrash()
    {
        MacAddress* mac = MacAddress::CreateInstanceOrNULL();
        if (mac == nullptr)
        {
            LOG_ERROR(logger, "FATAL ERROR OCCURED: FAILED TO READ MAC ADDRESS DUE TO DEVICE FAILURE OR OUT OF MEMORY");
            esp_restart();
        }
        ASSERT((mac != nullptr), "MAC ADDRESS REQUIRED AS AN IDENTIFIER FOR MODLINK MUST BE INSTANTIATED");

        ESP32FS* esp32FS = ESP32FS::CreateInstanceOrNULL();
        if (esp32FS == nullptr)
        {
            LOG_ERROR(logger, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");
            esp_restart();
        }
        ASSERT((esp32FS != nullptr), "ESP32FS REQUIRED AS A BASE FILE SYSTEM FOR MODLINK MUST BE INSTANTIATED");
        
        /**
         * @todo LittleFS 파티션의 포맷 여부를 로우 레벨 API로 확인해야 합니다.
         * @details 현재는 파티션 마운트에 실패할 경우 파티션을 자동으로 포맷하도록 개발하였습니다.
         *          다만, 일시적인 하드웨어 실패가 발생한 경우에도 파티션이 포맷되는 문제가 있습니다.
         */
        Status ret = esp32FS->Begin(true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR OCCURED: FAILED TO MOUNT ESP32 FILE SYSTEM TO OPERATING SYSTEM");
            esp_restart();
        }
        ASSERT((ret == Status::Code::GOOD), "ESP32FS REQUIRED AS A BASE FILE SYSTEM FOR MODLINK MUST BE MOUNTED");
        /*MUFFIN 프레임워크가 동작하기 위한 최소한의 조건이 만족되었습니다.*/
    }

    Status Initializer::Configure()
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        Status ret = esp32FS.DoesExist(JARVIS_FILE_PATH);

        if (ret == Status::Code::GOOD)
        {
            return configureWithJarvis();
        }
        else
        {
            return configureWithoutJarvis();
        }
    }

    Status Initializer::configureWithoutJarvis()
    {
        jarvis::config::CatM1 config;
        config.SetModel(jarvis::md_e::LM5);
        config.SetCounty(jarvis::ctry_e::KOREA);

        Status ret = InitCatM1(&config);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1");
            return ret;
        }

        ret = InitCatHTTP();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatHTTP");
            return ret;
        }
        
        ret = ConnectToBroker();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO THE BROKER");
            return ret;
        }
        
        return ret;
    }

    Status Initializer::configureWithJarvis()
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        fs::File file = esp32FS.Open(JARVIS_FILE_PATH);
        std::string payload;

        while (file.available() > 0)
        {
            payload += file.read();
        }

        LOG_DEBUG(logger, "%s", payload.c_str());
        
        JSON json;
        JsonDocument doc;
        Status ret = json.Deserialize(payload, &doc);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        Jarvis* jarvis = Jarvis::CreateInstanceOrNULL();
        jarvis->Validate(doc);
        ApplyJarvisTask();

        return Status(Status::Code::GOOD);
    }
}