/**
 * @file Initializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크의 초기화를 담당하는 클래스를 정의합니다.
 * 
 * @date 2025-01-13
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




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
#include "Protocol/SPEAR/SPEAR.h"

#include "Storage/ESP32FS/ESP32FS.h"




namespace muffin {
    
    Initializer::Initializer()
    {
    }
    
    Initializer::~Initializer()
    {
    }
    
    void Initializer::StartOrCrash()
    {
        /**
         * @todo LittleFS 파티션의 포맷 여부를 로우 레벨 API로 확인해야 합니다.
         * @details 현재는 파티션 마운트에 실패할 경우 파티션을 자동으로 포맷하도록 개발하였습니다.
         *          다만, 일시적인 하드웨어 실패가 발생한 경우에도 파티션이 포맷되는 문제가 있습니다.
         */
        Status ret = esp32FS.Begin(true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR OCCURED: FAILED TO MOUNT ESP32 FILE SYSTEM TO OPERATING SYSTEM");
            esp_restart();
        }
        ASSERT((ret == Status::Code::GOOD), "ESP32FS REQUIRED AS A BASE FILE SYSTEM FOR MODLINK MUST BE MOUNTED");
        /*MUFFIN 프레임워크가 동작하기 위한 최소한의 조건이 만족되었습니다.*/
    }

    Status Initializer::Configure(const bool hasJARVIS, const bool hasOTA)
    {
        if (hasJARVIS || hasOTA)
        {
            return configureWithoutJarvis(hasJARVIS);
        }
        
        Status ret = esp32FS.DoesExist(JARVIS_FILE_PATH);
        if (ret == Status::Code::GOOD)
        {
            return configureWithJarvis();
        }
        else
        {
            return configureWithoutJarvis(hasJARVIS);
        }
    }

    Status Initializer::configureWithoutJarvis(const bool hasJARVIS)
    {
    //     jarvis::config::CatM1 config;
    //     config.SetModel(jarvis::md_e::LM5);
    //     config.SetCounty(jarvis::ctry_e::KOREA);

    //     Status ret = InitCatM1(&config);
    //     if (ret != Status::Code::GOOD)
    //     {
    //         LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1");
    //         return ret;
    //     }
    // #if !defined(CATFS)
    //     ret = InitCatHTTP();
    //     if (ret != Status::Code::GOOD)
    //     {
    //         LOG_ERROR(logger, "FAILED TO INITIALIZE CatHTTP");
    //         return ret;
    //     }
        
        // ret = ConnectToBroker();
    //     if (ret != Status::Code::GOOD)
    //     {
    //         LOG_ERROR(logger, "FAILED TO CONNECT TO THE BROKER");
    //         return ret;
    //     }
    // #endif
    
    //     StartCatM1Task();

        

        jarvis::config::Ethernet config;
        config.SetDHCP(true);

        ethernet = new Ethernet();
        ethernet->Init();
        ethernet->Config(&config);
        ethernet->Connect();
        ethernet->SyncWithNTP();

        Status ret = ConnectToBrokerEthernet(); 
        StartEthernetTask();

        if (hasJARVIS)
        {
            core.StartJarvisTask();
        }
        
        return ret;
    }

    Status Initializer::configureWithJarvis()
    {
        LOG_WARNING(logger, "Config Start: %u Bytes", ESP.getFreeHeap());

        fs::File file = esp32FS.Open(JARVIS_FILE_PATH);
        std::string payload;

        while (file.available() > 0)
        {
            payload += file.read();
        }

        file.close();
        {
            JSON json;
            JsonDocument doc;
            Status ret = json.Deserialize(payload, &doc);
            if (ret != Status::Code::GOOD)
            {
                return ret;
            }

            payload.clear();
            payload.shrink_to_fit();

            /**
             * @todo DEMO용 디바이스들은 "fmt"로 저장되어있기 때문에 처리하기위해 임시로 구현하였음 1.2.0 버전 이후로 삭제 예정
             * 
             */
            JsonArray operation = doc["cnt"]["op"];
            for(JsonObject op : operation)
            {
                if(op.containsKey("fmt"))
                {
                    const bool value = op["fmt"].as<bool>();
                    op["rst"] = value;
                    op.remove("fmt");

                    std::string Updatepayload;
                    serializeJson(doc, Updatepayload);
                    file = esp32FS.Open(JARVIS_FILE_PATH, "w", true);
                    for (size_t i = 0; i < Updatepayload.length(); ++i)
                    {
                        file.write(Updatepayload[i]);
                    }
                    file.close();
                }
            }
            
            Jarvis* jarvis = Jarvis::CreateInstanceOrNULL();
            jarvis->Validate(doc);
        }
        ApplyJarvisTask();

        LOG_WARNING(logger, "Config Finished: %u Bytes", ESP.getFreeHeap());

        return Status(Status::Code::GOOD);
    }
}