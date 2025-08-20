/**
 * @file InitializeNetworkService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 초기화 함수를 정의합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/EthernetFactory.h"
#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"
#if defined(MB10) || defined(MT10)
    #include "Protocol/SPEAR/SPEAR.h"
#endif
#include "Protocol/MQTT/IMQTT.h"
 
TaskHandle_t xTaskLteMonitorHandle = NULL;



namespace muffin {
 
    /**
     * @btodo [담당자] 김주성 전임연구원
     *        [작업 상세]
     *            호출자가 없는데 삭제해도 되는 것인지 확인한
     *            다음 불필요한 함수일 경우 삭제 요망
     */
    void StopLteMonitorTask()
    {
        if (xTaskLteMonitorHandle == NULL)
        {
            LOG_WARNING(logger, "NO MODBUS LTE MONITOR TASK TO STOP!");
            return;
        }
        
        LOG_ERROR(logger, "STOPPED LTE MONITOR RTU TASK");
        vTaskDelete(xTaskLteMonitorHandle);
        xTaskLteMonitorHandle = NULL;
    }


    void implLteMonitorTask(void* pvParameters)
    {
        uint32_t statusReportMillis = millis(); 

        while (true)
        {
        if ((millis() - statusReportMillis) > (590 * SECOND_IN_MILLIS))
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);

                LOG_DEBUG(logger, "[LTE Monitor Task] Stack Remaind: %u Bytes", RemainedStackSize);
                
                deviceStatus.SetTaskRemainedStack(task_name_e::CATM1_MONITORING_TASK, RemainedStackSize);
            }
            if (catM1->IsConnected() == false)
            {
                LOG_WARNING(logger, "LTE Cat.M1 HAS LOST CONNECTION");
                mqttClient->ResetTEMP();
                catM1->Reconnect();
                InitCatM1Service();
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);   
        }

    }
 

    Status startTaskMonitoringCatM1()
    {
        if (xTaskLteMonitorHandle != NULL)
        {
            LOG_WARNING(logger, "LTE MONITORING TASK HAS ALREADY STARTED");
            return Status(Status::Code::GOOD);
        }

        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implLteMonitorTask,      // Function to be run inside of the task
            "implLteMonitorTask",    // The identifier of this task for men
            4 * KILLOBYTE,		    // Stack memory size to allocate
            NULL,			        // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskLteMonitorHandle,       // The identifier of this task for machines
            0				        // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Modbus TCP task has been started");
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


        if (xTaskLteMonitorHandle == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE LTE MONITORING TASK");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        else
        {
            LOG_INFO(logger, "Created LTE monitoring task");
        }

        LOG_INFO(logger, "Started CatM1 monitoring task");
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

#if !defined(MODLINK_L)
    Status InitEthernetService()
    {
        if (jvs::config::embeddedEthernet == nullptr)
        {
            LOG_DEBUG(logger, "Ethernet is not configured");
            return Status(Status::Code::GOOD);
        }
        
        INetwork* ethernet = EthernetFactory::Create();
        if (ethernet == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR ETHERNET INTERFACE");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
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
        LOG_INFO(logger, "Initialized Ethernet interface");

        ret = ethernet->Config(jvs::config::embeddedEthernet);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE ETHERNET CIN");
            return ret;
        }
        LOG_INFO(logger, "Configured ethernet CIN");

        ret = ethernet->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT ETHERNET");
            return ret;
        }
        LOG_INFO(logger, "Ethernet has connected");
    
        if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            LOG_INFO(logger,"Initialized ethernet interface");
            return Status(Status::Code::GOOD);
        }
        
        const uint32_t startedMillis = millis();
        do
        {
            ret = ethernet->SyncNTP();
            if ((millis() - startedMillis) > 10*SECOND_IN_MILLIS)
            {
                LOG_ERROR(logger, "FAILED TO SYNC WITH NTP SERVER. DEVICE WILL BE RESTARTED");
                #if defined(MB10) || defined(MT10)
                    spear.Reset();
                #endif
                esp_restart();
            }
        } while (ret != Status::Code::GOOD);
        LOG_INFO(logger, "Synchronized with NTP server");

        LOG_INFO(logger,"Initialized ethernet interface");
        return ret;
    }
#endif
}