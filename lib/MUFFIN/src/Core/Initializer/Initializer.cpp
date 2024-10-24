/**
 * @file Initializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크의 초기화를 담당하는 클래스를 정의합니다.
 * 
 * @date 2024-10-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Core/Initializer/Initializer.h"
#include "Core/Include/Helper.h"
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
        MacAddress* mac = MacAddress::GetInstanceOrNULL();
        if (mac == nullptr)
        {
            ASSERT((mac != nullptr), "FATAL ERROR OCCURED: FAILED TO READ MAC ADDRESS DUE TO MEMORY OR DEVICE FAILURE");
            LOG_ERROR(logger, "FAILED TO READ MAC ADDRESS DUE TO MEMORY OR DEVICE FAILURE");
            esp_restart();
        }
        ASSERT((mac != nullptr), "[FATAL ERROR] MAC ADDRESS REQUIRED AS IDENTIFIERS FOR MODLINK MUST BE INSTANTIATED");

        ESP32FS* esp32FS = ESP32FS::GetInstanceOrNULL();
        if (esp32FS == nullptr)
        {
            ASSERT((esp32FS != nullptr), "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");
            esp_restart();
        }
        ASSERT((esp32FS != nullptr), "[FATAL ERROR] ESP32FS REQUIRED AS A BASE FILE SYSTEM FOR MODLINK MUST BE INSTANTIATED");
        
        /**
         * @todo LittleFS 파티션의 포맷 여부를 로우 레벨 API로 확인해야 합니다.
         * @details 현재는 파티션 마운트에 실패할 경우 파티션을 자동으로 포맷하게
         *          코드를 작성하였습니다. 다만, 일시적인 하드웨어 실패가 발생한
         *          경우에도 파티션이 포맷되는 문제가 있습니다.
         */
        Status ret = esp32FS->Begin(true);
        if (ret != Status::Code::GOOD)
        {
            ASSERT(false, "FATAL ERROR OCCURED: FAILED TO MOUNT ESP32 FILE SYSTEM TO OPERATING SYSTEM");
            LOG_ERROR(logger, "FAILED TO MOUNT ESP32 FILE SYSTEM TO THE OS");
            esp_restart();
        }
        ASSERT((ret == Status::Code::GOOD), "[FATAL ERROR] ESP32FS REQUIRED AS A BASE FILE SYSTEM FOR MODLINK MUST BE MOUNTED");
        /*MUFFIN 프레임워크가 동작하기 위한 최소한의 조건이 만족되었습니다.*/
    }

    Status Initializer::Configure()
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        if (esp32FS.DoesExist("/jarvis/config.json") == Status::Code::GOOD)
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
    #if defined(DEBUG)
        mqtt::BrokerInfo info(
            MacAddress::GetEthernet(),
            "mqtt.vitcon.iotops.opsnow.com",
            8883,
            40,
            mqtt::socket_e::SOCKET_0,
            "vitcon",
            "tkfkdgo5!@#$"
        );
    #else
        mqtt::BrokerInfo info(MacAddress::GetEthernet());
    #endif

        jarvis::config::CatM1 config;
        config.SetModel(jarvis::md_e::LM5);
        config.SetCounty(jarvis::ctry_e::KOREA);

        if (mIsMqttTopicCreated == false)
        {
            if (mqtt::Topic::CreateTopic(MacAddress::GetEthernet()) == false)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT TOPIC");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            mIsMqttTopicCreated = true;
        }
        /*LTE Cat.M1 모뎀 사용에 필요한 설정 정보를 생성하는데 성공했습니다.*/



        if (mIsCatM1Connected == false)
        {
            CatM1* catM1 = CatM1::GetInstanceOrNULL();
            if (catM1 == nullptr)
            {
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            Status ret = catM1->Config(&config);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE CatM1 MODULE");
                return ret;
            }

            ret = catM1->Init();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO INIT CatM1 MODULE");
                return ret;
            }

            ret = catM1->Connect();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONNECT CatM1 MODULE TO ISP");
                return ret;
            }

            while (catM1->IsConnected() == false)
            {
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            ret = SyncWithNTP(jarvis::snic_e::LTE_CatM1);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SYNC WITH NTP SERVER: %s", ret.c_str());
                return ret;
            }

            mIsCatM1Connected = true;
        }
        /*LTE Cat.M1 모뎀이 인터넷에 연결되었으며 사용 가능합니다.*/

        

        if (mIsCatMQTTConnected == false)
        {
            mqtt::CIA* cia = mqtt::CIA::GetInstanceOrNULL();
            if (cia == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT CIA");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            mqtt::CDO* cdo = mqtt::CDO::GetInstanceOrNULL();
            if (cdo == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT CDO");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            CatM1& catM1 = CatM1::GetInstance();
            mqtt::CatMQTT* catMQTT = mqtt::CatMQTT::GetInstanceOrNULL(catM1, info);
            if (catMQTT == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatMQTT");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            Status ret = catMQTT->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO INIT CatMQTT: %s", ret.c_str());
                return ret;
            }
            
            ret = catMQTT->Connect();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONNECT TO THE MQTT BROKER: %s", ret.c_str());
                return ret;
            }

            while (catMQTT->IsConnected() != Status::Code::GOOD)
            {
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            mqtt::Message topicJARVIS(mqtt::topic_e::JARVIS_REQUEST, "");
            mqtt::Message topicREMOTE_CONTROL_REQUEST(mqtt::topic_e::REMOTE_CONTROL_REQUEST, "");
            /**
             * @todo JARVIS REQUEST, REMOTE CONTROL 토픽 구독 완료, 추가로 구독해야하는 토픽이 있나?
             */
            std::vector<mqtt::Message> vectorTopicsToSubscribe;
            ret = EmplaceBack(std::move(topicJARVIS), &vectorTopicsToSubscribe);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE TOPICS TO SUBSCRIBE: %s", ret.c_str());
                return ret;
            }

            ret = EmplaceBack(std::move(topicREMOTE_CONTROL_REQUEST), &vectorTopicsToSubscribe);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE TOPICS TO SUBSCRIBE: %s", ret.c_str());
                return ret;
            }

            ret = catMQTT->Subscribe(vectorTopicsToSubscribe);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SUBSCRIBE MQTT TOPICS: %s", ret.c_str());
                return ret;
            }

            mIsCatMQTTConnected = true;
        }
        /*LTE Cat.M1 모뎀이 MQTT 브로커에 연결되었으며 사용 가능합니다.*/
        


        if (mIsCatHTTPConfigured == false)
        {
            CatM1& catM1 = CatM1::GetInstance();
            http::CatHTTP* catHTTP = http::CatHTTP::GetInstanceOrNULL(catM1);
            if (catHTTP == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatHTTP");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            Status ret = catHTTP->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_1);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO INIT CatHTTP: %s", ret.c_str());
                return ret;
            }

            mIsCatHTTPConfigured = true;
        }
        /*LTE Cat.M1 모뎀의 HTTP 프로토콜 기능이 설정되었으며 사용 가능합니다.*/
        
        return Status(Status::Code::GOOD);
    }

    Status Initializer::configureWithJarvis()
    {
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}