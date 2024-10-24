/**
 * @file NetworkTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 사용과 관련된 태스크를 정의합니다.
 * 
 * @date 2024-10-20
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
#include "IM/MacAddress/MacAddress.h"
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

    static bool s_IsMqttTopicCreated   = false;
    static bool s_IsCatM1Connected     = false;
    static bool s_IsCatMQTTConnected   = false;
    static bool s_IsCatHTTPConfigured  = false;


    Status initializeCatM1()
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

        if (s_IsMqttTopicCreated == false)
        {
            if (mqtt::Topic::CreateTopic(MacAddress::GetEthernet()) == false)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT TOPIC");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            s_IsMqttTopicCreated = true;
        }
        /*LTE Cat.M1 모뎀 사용에 필요한 설정 정보를 생성하는데 성공했습니다.*/


        /**
         * @todo LTE Cat.M1 부팅 후 Mqtt 브로커 재 연결 시 토픽 재구독 기능이 추가되어야 합니다.
         * 
         */
        if (s_IsCatM1Connected == false)
        {
            CatM1& catM1 = CatM1::GetInstance();
            if (catM1.IsConnected() == false)
            {
                Status ret = catM1.Init();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO INIT CatM1 MODULE");
                    return ret;
                }

                ret = catM1.Connect();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO CONNECT CatM1 MODULE TO ISP");
                    return ret;
                }

                while (catM1.IsConnected() == false)
                {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }

                ret = SyncWithNTP(jarvis::snic_e::LTE_CatM1);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO SYNC WITH NTP SERVER: %s", ret.c_str());
                    return ret;
                }
            }
            else
            {
                LOG_DEBUG(logger, "CatM1 is already initialized");
            }
            s_IsCatM1Connected = true;
        }
        /*LTE Cat.M1 모뎀이 인터넷에 연결되었으며 사용 가능합니다.*/

        

        if (s_IsCatMQTTConnected == false)
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

            if (catMQTT->IsConnected() != Status::Code::GOOD)
            {
                Status ret = catMQTT->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO INIT CatMQTT: %s", ret.c_str());
                    return ret;
                }

                while (catMQTT->IsConnected() != Status::Code::GOOD)
                {
                    ret = catMQTT->Connect();
                    if (ret != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger, "FAILED TO CONNECT TO THE MQTT BROKER: %s", ret.c_str());
                        return ret;
                    }
                
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
            }
            else
            {
                LOG_DEBUG(logger, "CatMQTT is already initialized");
            }
            s_IsCatMQTTConnected = true;
        }
        /*LTE Cat.M1 모뎀이 MQTT 브로커에 연결되었으며 사용 가능합니다.*/
        


        if (s_IsCatHTTPConfigured == false)
        {
            CatM1& catM1 = CatM1::GetInstance();
            http::CatHTTP* catHTTP = http::CatHTTP::GetInstanceOrNULL(catM1);
            if (catHTTP == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatHTTP");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            if (catHTTP->IsInitialized() != Status::Code::GOOD)
            {
                Status ret = catHTTP->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_1);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO INIT CatHTTP: %s", ret.c_str());
                    return ret;
                }
            }
            else
            {
                LOG_DEBUG(logger, "CatHTTP is already initialized");
            }
            s_IsCatHTTPConfigured = true;
        }
        /*LTE Cat.M1 모뎀의 HTTP 프로토콜 기능이 설정되었으며 사용 가능합니다.*/
        
        return Status(Status::Code::GOOD);
    }

    void implCatM1Task(void* pvParameters)
    {
        constexpr uint16_t SECOND_IN_MILLIS = 1000;
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif
        
        while (initializeCatM1() != Status::Code::GOOD)
        {
            vTaskDelay(10 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: CatM1] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }

        CatM1& catM1 = CatM1::GetInstance();
        mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
        
        bool eventLteDisconnected  = false;
        time_t timeLteDisconnected = 0;

        bool eventMqttDisconnected  = false;
        time_t timeMqttDisconnected = 0;

        constexpr time_t TIME_TO_RESET_LTE_MODEM = 10 * SECOND_IN_MILLIS;


        while (true)
        {
            if (catM1.IsConnected() == false && eventLteDisconnected == false)
            {
                LOG_WARNING(logger, "LTE Cat.M1 LOST CONNECTION");
                timeLteDisconnected  = GetTimestamp();
                eventLteDisconnected = true;
            }
            else if (catM1.IsConnected() == true && eventLteDisconnected == true)
            {
                LOG_INFO(logger, "LTE Cat.M1 IS BACK ONLINE");
                timeLteDisconnected  = 0;
                eventLteDisconnected = false;
            }
            
            if (catMqtt.IsConnected() != Status::Code::GOOD && eventMqttDisconnected == false)
            {
                LOG_WARNING(logger, "CatMQTT LOST CONNECTION");
                timeMqttDisconnected  = GetTimestamp();
                eventMqttDisconnected = true;
            }
            else if (catM1.IsConnected() == true && eventMqttDisconnected == true)
            {
                LOG_INFO(logger, "CatMQTT IS BACK ONLINE");
                timeMqttDisconnected  = 0;
                eventMqttDisconnected = false;
            }


            if (eventLteDisconnected == true)
            {
                if ((GetTimestamp() - timeLteDisconnected) > TIME_TO_RESET_LTE_MODEM)
                {
                    LOG_INFO(logger, "START TO RESET THE LTE Cat.M1 MODEM");
                
                    catM1.Reconnect();
                    s_IsCatM1Connected     = false;
                    s_IsCatMQTTConnected   = false;
                    s_IsCatHTTPConfigured  = false;
                    
                    while (initializeCatM1() != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger, "FAILED TO REINITIALIZING THE LTE Cat.M1 MODEM");
                        vTaskDelay(10 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                    }

                    eventLteDisconnected  = false;
                    eventMqttDisconnected = false;
                    timeLteDisconnected   = 0;
                    timeMqttDisconnected  = 0;
                }
            }
            
            if (eventLteDisconnected == false && eventMqttDisconnected == true)
            {
                while (catMqtt.Connect() != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO RECONNECT THE CatMQTT TO THE BROKER");
                    vTaskDelay(10 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                }

                eventMqttDisconnected = false;
                timeMqttDisconnected  = 0;
            }


            vTaskDelay(5 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: CatM1] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
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
        
        /**
         * @todo 향후 태스크의 메모리 사용량을 보고 스택 메모리 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implCatM1Task,      // Function to be run inside of the task
            "implCatM1Task",    // The identifier of this task for men
            4096,			   // Stack memory size to allocate
            NULL,			   // Task parameters to be passed to the function
            0,				   // Task Priority for scheduling
            &xTaskCatM1Handle,  // The identifier of this task for machines
            0				   // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The MQTT task has been started");
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