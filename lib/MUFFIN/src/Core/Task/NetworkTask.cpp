/**
 * @file NetworkTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 사용과 관련된 태스크를 정의합니다.
 * 
 * @date 2024-10-30
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Core/Include/Helper.h"
#include "Core/Task/NetworkTask.h"
#include "IM/MacAddress/MacAddress.h"
#include "Jarvis/Include/Base.h"
#include "NetworkTask.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/RequestHeader.h"



namespace muffin {

    TaskHandle_t xTaskCatM1Handle;

    static bool s_IsMqttTopicCreated      = false;
    static bool s_IsCatM1Connected        = false;
    static bool s_IsCatMqttInitialized    = false;
    static bool s_IsCatMQTTConnected      = false;
    static bool s_IsCatHttpInitialized    = false;
    static bool s_IsCatMqttTopicSubscribed = false;


    Status InitCatM1(jarvis::config::CatM1* cin)
    {
        mqtt::CIA* cia = mqtt::CIA::CreateInstanceOrNULL();
        if (cia == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CIA DUE TO OUT OF MEMORY");
            esp_restart();
        }

        mqtt::CDO* cdo = mqtt::CDO::CreateInstanceOrNULL();
        if (cdo == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CDO DUE TO OUT OF MEMORY");
            esp_restart();
        }
        
        CatM1* catM1 = CatM1::CreateInstanceOrNULL();
        if (catM1 == nullptr)
        {
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        const auto retrievedCIN = catM1->RetrieveConfig();
        const bool isConfigured = (retrievedCIN.first == true) && (retrievedCIN.second == *cin);
        
        if ((isConfigured == true) && (s_IsCatM1Connected == true))
        {
            LOG_INFO(logger, "LTE Cat.M1 modem is already initialized");
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_INFO(logger, "New configuration for LTE Cat.M1 modem");

            s_IsMqttTopicCreated      = false;
            s_IsCatM1Connected        = false;
            s_IsCatMqttInitialized    = false;
            s_IsCatMQTTConnected      = false;
            s_IsCatHttpInitialized    = false;
            s_IsCatMqttTopicSubscribed = false;
        }

        jarvis::config::Base* baseCIN = static_cast<jarvis::config::Base*>(cin);
        Status ret = catM1->Config(baseCIN);
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

        s_IsCatM1Connected = true;
        return ret;
    }

    
    Status InitCatHTTP()
    {
        if (s_IsCatM1Connected == false)
        {
            LOG_ERROR(logger, "FAILED TO INIT CatHTTP: LTE MODEM IS NOT CONNECTED");
            return Status(Status::Code::BAD_NO_CONTINUATION_POINTS);
        }

        if (s_IsCatHttpInitialized == true)
        {
            LOG_WARNING(logger, "NOTHING TO DO: ALREADY INITIALIZED THE CatHTTP");
            return Status(Status::Code::GOOD);
        }

        CatM1& catM1 = CatM1::GetInstance();
        http::CatHTTP* catHttp = http::CatHTTP::CreateInstanceOrNULL(catM1);
        if (catHttp == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE CatHTTP DUE TO OUT OF MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        if (catHttp->IsInitialized() == Status::Code::GOOD)
        {
            // LOG_DEBUG(logger, "CatHTTP is already initialized");
        }
        
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }
        
        catHttp->OnEventReset();
        Status ret = catHttp->Init(mutexHandle.second, network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_1);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INIT CatHTTP: %s", ret.c_str());
            catM1.ReleaseMutex();
            return ret;
        }
        s_IsCatHttpInitialized = true;
        
        catM1.ReleaseMutex();
        return Status(Status::Code::GOOD);
    }


    Status ConnectToBroker()
    {
        mqtt::CIA* cia = mqtt::CIA::CreateInstanceOrNULL();
        if (cia == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CIA DUE TO OUT OF MEMORY");
            esp_restart();
        }

        mqtt::CDO* cdo = mqtt::CDO::CreateInstanceOrNULL();
        if (cdo == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CDO DUE TO OUT OF MEMORY");
            esp_restart();
        }
        
        if (s_IsCatM1Connected == false)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO BROKER: LTE MODEM IS NOT CONNECTED");
            return Status(Status::Code::BAD_NO_CONTINUATION_POINTS);
        }

        if (s_IsMqttTopicCreated == false)
        {
            if (mqtt::Topic::CreateTopic(MacAddress::GetEthernet()) == false)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT TOPIC");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            s_IsMqttTopicCreated = true;
        }

        /**
         * @todo LTE Cat.M1 부팅 후 브로커 재연결 시 토픽 재구독 기능이 추가되어야 합니다.
         */
        if (s_IsCatMQTTConnected == true)
        {
            LOG_WARNING(logger, "NOTHING TO DO: ALREADY CONNECTED TO THE BROKER");
            return Status(Status::Code::GOOD);
        }

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
        // mqtt::BrokerInfo info(MacAddress::GetEthernet());
        mqtt::BrokerInfo info(
            MacAddress::GetEthernet(),
            "mqtt.vitcon.iotops.opsnow.com",
            8883,
            40,
            mqtt::socket_e::SOCKET_0,
            "vitcon",
            "tkfkdgo5!@#$"
        );
    #endif
        
        char buffer[32] = { '\0' };
        sprintf(buffer, "%s,disconnected", MacAddress::GetEthernet());
        mqtt::Message lwt(mqtt::topic_e::LAST_WILL, buffer);
        
        CatM1& catM1 = CatM1::GetInstance();
        mqtt::CatMQTT* catMqtt = mqtt::CatMQTT::CreateInstanceOrNULL(catM1, info, lwt);
        if (catMqtt == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE CatMQTT DUE TO OUT OF MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        if (s_IsCatMqttInitialized == false)
        {
            /**
             * @todo onEventReset 코드로 인해 LWT 설정이 지워지는 문제로 인해 
             *       setLastWill 함수 내부에 임시로 코드를 변경하는 해킹을 넣었습니다.
             *       향후에는 전반적인 로직을 수정해야 합니다.
             */
            catMqtt->OnEventReset();
            Status ret = catMqtt->Init(mutexHandle.second, network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO INIT CatMQTT: %s", ret.c_str());
                catM1.ReleaseMutex();
                return ret;
            }
            s_IsCatMqttInitialized = true;
        }

        Status ret = catMqtt->Connect(mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO THE MQTT BROKER: %s", ret.c_str());
            catM1.ReleaseMutex();
            return ret;
        }

        if (s_IsCatMqttTopicSubscribed == false)
        {
            mqtt::Message topicJARVIS(mqtt::topic_e::JARVIS_REQUEST, "");
            mqtt::Message topicRemoteControl(mqtt::topic_e::REMOTE_CONTROL_REQUEST, "");
            mqtt::Message topicFotaUpdate(mqtt::topic_e::FOTA_UPDATE, "");
            std::vector<mqtt::Message> vectorTopicsToSubscribe;
            Status retTopic01 = EmplaceBack(std::move(topicJARVIS), &vectorTopicsToSubscribe);
            Status retTopic02 = EmplaceBack(std::move(topicRemoteControl), &vectorTopicsToSubscribe);
            Status retTopic03 = EmplaceBack(std::move(topicFotaUpdate), &vectorTopicsToSubscribe);
            if ((retTopic01 != Status::Code::GOOD) || (retTopic02 != Status::Code::GOOD) 
            || (retTopic03 != Status::Code::GOOD))
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE TOPICS TO SUBSCRIBE: %s, %s, %s", 
                    retTopic01.c_str(), retTopic02.c_str(), retTopic03.c_str());
                catM1.ReleaseMutex();
                s_IsCatMqttTopicSubscribed = false;
                return ret;
            }

            ret = catMqtt->Subscribe(mutexHandle.second, vectorTopicsToSubscribe);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SUBSCRIBE MQTT TOPICS: %s", ret.c_str());
                catM1.ReleaseMutex();
                s_IsCatMqttTopicSubscribed = false;
                return ret;
            }

            s_IsCatMqttTopicSubscribed = true;
        }
        


        s_IsCatMQTTConnected = true;
        catM1.ReleaseMutex();
        return ret;
    }


    void implCatM1Task(void* pvParameters)
    {
        constexpr uint16_t SECOND_IN_MILLIS = 1000;
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        CatM1& catM1 = CatM1::GetInstance();
        mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
        
        while (true)
        {
            if (catM1.IsConnected() == false)
            {
                LOG_WARNING(logger, "LTE Cat.M1 LOST CONNECTION");
                catM1.Reconnect();
                s_IsMqttTopicCreated     = false;
                s_IsCatM1Connected       = false;
                s_IsCatMqttInitialized   = false;
                s_IsCatMQTTConnected     = false;
                s_IsCatHttpInitialized   = false;

                jarvis::config::CatM1 retrievedCIN = catM1.RetrieveConfig().second;
                while (InitCatM1(&retrievedCIN) != Status::Code::GOOD)
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                InitCatHTTP();
                ConnectToBroker();
            }
            
            
            if ((catMqtt.IsConnected() != Status::Code::GOOD) || (s_IsCatMQTTConnected == false))
            {
                LOG_WARNING(logger, "CatMQTT LOST CONNECTION");
                ConnectToBroker();
            }


            if (s_IsCatHttpInitialized == false)
            {
                InitCatHTTP();
            }


            vTaskDelay(10 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                // LOG_DEBUG(logger, "[TASK: CatM1] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }

    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void StartCatM1Task()
    {
        if (xTaskCatM1Handle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        mqtt::CIA* cia = mqtt::CIA::CreateInstanceOrNULL();
        if (cia == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CIA DUE TO OUT OF MEMORY");
            esp_restart();
        }

        mqtt::CDO* cdo = mqtt::CDO::CreateInstanceOrNULL();
        if (cdo == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MQTT CDO DUE TO OUT OF MEMORY");
            esp_restart();
        }
        
        /**
         * @todo 향후 태스크의 메모리 사용량을 보고 스택 메모리 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implCatM1Task,      // Function to be run inside of the task
            "CatM1Task",        // The identifier of this task for men
            4096,			    // Stack memory size to allocate
            NULL,			    // Task parameters to be passed to the function
            0,				    // Task Priority for scheduling
            &xTaskCatM1Handle,  // The identifier of this task for machines
            0				    // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The CatM1 task has been started");
            // return Status(Status::Code::GOOD);
            break;

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            break;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
            break;

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            break;
        }
    }
}