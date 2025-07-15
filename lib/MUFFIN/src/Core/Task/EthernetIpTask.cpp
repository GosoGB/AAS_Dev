/**
 * @file EthernetIpTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Ethernet/IP 프로토콜로 데이터를 수집하는 태스크를 정의합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */


#if defined(MT11)

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Core/Core.h"
#include "Core/Task/PubTask.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"
#include "Core/Task/EthernetIpTask.h"
#include "Protocol/EthernetIP/EthernetIP.h"
#include "Protocol/EthernetIP/EthernetIpMutex.h"


namespace muffin {

    std::vector<ethernetIP::EthernetIP> EthernetIpVector;
    TaskHandle_t xTaskEthernetIpHandle = NULL;

    void implEthernetIpTask(void* pvParameter)
    {   
        uint32_t statusReportMillis = millis(); 
        while (true)
        {
        #if defined(DEBUG)
            if ((millis() - statusReportMillis) > (590 * SECOND_IN_MILLIS))
        #else
            if ((millis() - statusReportMillis) > (3550 * SECOND_IN_MILLIS))
        #endif
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
        
                LOG_DEBUG(logger, "[EthernetIpTask} Stack Remaind: %u Bytes", RemainedStackSize);

                deviceStatus.SetTaskRemainedStack(task_name_e::ETHERNET_IP_TASK, RemainedStackSize);
            }

            if (g_DaqTaskSetFlag.test(static_cast<uint8_t>(set_task_flag_e::ETHERNET_IP_TASK)))
            {
                continue;
            }

            
            
            for(auto& EthernetIp : EthernetIpVector)
            {
                if (xSemaphoreTake(xSemaphoreEthernetIP, 2000)  != pdTRUE)
                {
                    LOG_WARNING(logger, "[EthernetIP] THE READ MODULE IS BUSY. TRY LATER.");
                    continue;
                }

                if (!EthernetIp.mEipSession.client->connected())
                {
                    if (!EthernetIp.Connect())
                    {
                        LOG_ERROR(logger,"EthernetIp Client failed to connect!, serverIP : %s, serverPort: %d", EthernetIp.mEipSession.targetIP.toString().c_str(), EthernetIp.mEipSession.targetPort);
                        // EthernetIp.SetTimeoutError();     
                        EthernetIp.mEipSession.client->stop(); 
                        EthernetIp.mEipSession.connected = false;  
                        
                        xSemaphoreGive(xSemaphoreEthernetIP); 
                        continue;
                    } 
                }

                Status ret = EthernetIp.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }

                EthernetIp.mEipSession.client->stop(); 
                EthernetIp.mEipSession.connected = false;

                xSemaphoreGive(xSemaphoreEthernetIP);
            }
            

            g_DaqTaskSetFlag.set(static_cast<uint8_t>(set_task_flag_e::ETHERNET_IP_TASK));
            vTaskDelay(s_PollingIntervalInMillis / portTICK_PERIOD_MS);
        }
    }

    void StartEthernetIpTask()
    {
        if (xTaskEthernetIpHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }

        xSemaphoreEthernetIP = xSemaphoreCreateMutex();
        if (xSemaphoreEthernetIP == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE EthernetIp SEMAPHORE");
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해 태스크의 메모리 사용량에 따라
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implEthernetIpTask,      // Function to be run inside of the task
            "implEthernetIpTask",    // The identifier of this task for men
            10 * KILLOBYTE,          // Stack memory size to allocate
            NULL, // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskEthernetIpHandle,  // The identifier of this task for machines
            1				        // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The EthernetIp task has been started");
            g_DaqTaskEnableFlag.set(static_cast<uint8_t>(set_task_flag_e::ETHERNET_IP_TASK));
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

    void StopEthernetIpTask()
    {
        if (xTaskEthernetIpHandle == NULL)
        {
            LOG_WARNING(logger, "NO EthernetIp TASK TO STOP!");
            return;
        }
        g_DaqTaskEnableFlag.reset(static_cast<uint8_t>(set_task_flag_e::ETHERNET_IP_TASK));
        vTaskDelete(xTaskEthernetIpHandle);
        xTaskEthernetIpHandle = NULL;
        LOG_INFO(logger, "STOPPED THE EthernetIp TASK");
    }

    bool HasEthernetIpTask()
    {
        if (xTaskEthernetIpHandle == NULL)
        {
            return false;
        }
        else
        {
            return true;
        }
    }



}



#endif