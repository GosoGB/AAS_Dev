/**
 * @file InitializeNetworkService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 초기화 함수를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"
#include "Protocol/SPEAR/SPEAR.h"

static TimerHandle_t xTimer = NULL;



namespace muffin {

    void implLteTimerCallback()
    {
        if (catM1->IsConnected() == false)
        {
            // CatHTTP & CatMQTT에 연결 끊어졌다는 신호 줘야 함

            LOG_WARNING(logger, "LTE Cat.M1 HAS LOST CONNECTION");
            catM1->Reconnect();
            InitCatM1Service();

            // CatHTTP & CatMQTT 다시 설정을 잡도록 해줘야 함
            // InitCatHTTP();
            // ConnectToBroker();
        }
    }

    void vLteTimerCallback(TimerHandle_t xTimer)
    {
        configASSERT(xTimer);   // Optionally do something if xTimer is NULL
        implLteTimerCallback();
    }

    Status startTaskMonitoringCatM1()
    {
        if (xTimer != NULL)
        {
            return Status(Status::Code::GOOD);
        }
        
        xTimer = xTimerCreate(
            "lte_timer_loop",     // pcTimerName
            10*SECOND_IN_MILLIS,  // xTimerPeriod,
            pdTRUE,               // uxAutoReload,
            (void *)0,            // pvTimerID,
            vLteTimerCallback     // pxCallbackFunction
        );

        if (xTimer == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE TIMER FOR LTE MONITORING");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        else
        {
            LOG_INFO(logger, "Created a timer for LTE monitoring task");
            if (xTimerStart(xTimer, 0) != pdPASS)
            {
                LOG_ERROR(logger, "FAILED TO START TIMER FOR LTE MONITORING TASK");
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        LOG_INFO(logger, "Started CatM1 monitoring timer task");
        return Status(Status::Code::GOOD);
    }

    Status InitCatM1Service()
    {
        if (jvs::config::catM1 == nullptr)
        {
            LOG_DEBUG(logger, "CatM1 is not configured");
            return Status(Status::Code::GOOD);
        }
        
        if (catM1 == nullptr)
        {
            catM1 = new(std::nothrow) CatM1();
            if (catM1 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }

        if (catM1->IsConnected() == true)
        {
            return Status(Status::Code::GOOD);
        }
        
        Status ret = catM1->Config(jvs::config::catM1);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE CatM1 CIN");
            return ret;
        }
        LOG_INFO(logger,"Configured CatM1 CIN");

        ret = catM1->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1");
            return ret;
        }

        ret = catM1->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT CatM1 MODULE TO ISP");
            return ret;
        }
        LOG_INFO(logger,"CatM1 has connected");
        
        ret = startTaskMonitoringCatM1();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO START CatM1 MONITORING TASK");
            return ret;
        }
        LOG_INFO(logger,"Start to monitoring CatM1 modem");
        
        do
        {
            if (catM1->IsConnected() == false)
            {
                ret = Status::Code::BAD_NO_COMMUNICATION;
                goto RETRY;
            }
            
            ret = catM1->SyncNTP();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

        RETRY:
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        } while (ret != Status::Code::GOOD);
        LOG_INFO(logger, "Synchronized with NTP server");
        
        LOG_INFO(logger,"Initialized CatM1 interface");
        
        return ret;
    }

#if defined(MODLINK_T2) || defined(MODLINK_B)
    Status InitEthernetService()
    {
        if (jvs::config::ethernet == nullptr)
        {
            LOG_DEBUG(logger, "Ethernet is not configured");
            return Status(Status::Code::GOOD);
        }
        
        if (ethernet == nullptr)
        {
            ethernet = new(std::nothrow) Ethernet();
            if (ethernet == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }
        
        if (ethernet->IsConnected() == true)
        {
            return Status(Status::Code::GOOD);
        }

        Status ret = ethernet->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE ETHERNET");
            return ret;
        }
        
        ret = ethernet->Config(jvs::config::ethernet);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE ETHERNET CIN");
            return ret;
        }
        LOG_INFO(logger,"Configured ethernet CIN");

        ret = ethernet->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT ETHERNET");
            return ret;
        }
        LOG_INFO(logger,"Ethernet has connected");
        
        const uint32_t startedMillis = millis();
        do
        {
            ret = ethernet->SyncNTP();
            if ((millis() - startedMillis) > 10*SECOND_IN_MILLIS)
            {
                LOG_ERROR(logger, "FAILED TO SYNC WITH NTP SERVER. DEVICE WILL BE RESTARTED");
                spear.Reset();
                esp_restart();
            }
        } while (ret != Status::Code::GOOD);
        LOG_INFO(logger, "Synchronized with NTP server");

        LOG_INFO(logger,"Initialized ethernet interface");
        return ret;
    }
#endif

    Status InitWiFiService()
    {
        ASSERT((false), "Wi-Fi INTERFACE IS NOT SUPPORTED YET");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}