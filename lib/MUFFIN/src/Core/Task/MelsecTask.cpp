/**
 * @file MelsecTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Melsec 프로토콜로 데이터를 수집하는 태스크를 정의합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




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
#include "Core/Task/MelsecTask.h"
#include "Protocol/Melsec/MelsecMutex.h"

namespace muffin 
{

    std::vector<Melsec> MelsecVector;
    std::vector<Melsec> MelsecVectorDynamic;
    TaskHandle_t xTaskMelsecHandle = NULL;

    void implMelsecTask(void* pvParameter)
    {   
        uint32_t statusReportMillis = millis(); 
        while (true)
        {
        #if defined(DEBUG)
            if ((millis() - statusReportMillis) > (30 * SECOND_IN_MILLIS))
        #else
            if ((millis() - statusReportMillis) > (3550 * SECOND_IN_MILLIS))
        #endif
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
        
                LOG_INFO(logger, "[MelSecTask} Stack Remaind: %u Bytes", RemainedStackSize);

                deviceStatus.SetTaskRemainedStack(task_name_e::MELSEC_TASK, RemainedStackSize);
            }

            if (g_DaqTaskSetFlag.test(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK)))
            {
                continue;
            }

            for(auto& melsec : MelsecVector)
            {
                if (!melsec.mMelsecClient->Connected())
                {
                    if (!melsec.Connect())
                    {
                        LOG_ERROR(logger,"melsec Client failed to connect!, serverIP : %s, serverPort: %d", melsec.GetServerIP().toString().c_str(), melsec.GetServerPort());
                        melsec.SetTimeoutError();           
                        continue;
                    } 
                }
                
                Status ret = melsec.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }

                melsec.mMelsecClient->Close();
            }

        #if defined(MT11)
            for(auto& melsec : MelsecVectorDynamic)
            {
                if (!melsec.Connect())
                {
                    LOG_ERROR(logger,"melsec Client failed to connect!, serverIP : %s, serverPort: %d", melsec.GetServerIP().toString().c_str(), melsec.GetServerPort());
                    melsec.SetTimeoutError();
                    continue;
                } 
                else
                {
                    LOG_DEBUG(logger,"melsec Client connected");
                }

                Status ret = melsec.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }

                melsec.mMelsecClient->Close();
                
            }
        #endif
            g_DaqTaskSetFlag.set(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK));
            vTaskDelay(s_PollingIntervalInMillis / portTICK_PERIOD_MS);
        }
    }

    void StartMelsecTask()
    {
        if (xTaskMelsecHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }

        xSemaphoreMelsec = xSemaphoreCreateMutex();
        if (xSemaphoreMelsec == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MELSEC SEMAPHORE");
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해 태스크의 메모리 사용량에 따라
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implMelsecTask,      // Function to be run inside of the task
            "implMelsecTask",    // The identifier of this task for men
    #if defined(MT11)
            10 * KILLOBYTE,          // Stack memory size to allocate
    #else
            5 * KILLOBYTE,          // Stack memory size to allocate
    #endif
            NULL, // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskMelsecHandle,  // The identifier of this task for machines
            1				        // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Melsec task has been started");
            g_DaqTaskEnableFlag.set(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK));
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

    void StopMelsecTask()
    {
        if (xTaskMelsecHandle == NULL)
        {
            LOG_WARNING(logger, "NO MELSEC TASK TO STOP!");
            return;
        }
        g_DaqTaskEnableFlag.reset(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK));
        vTaskDelete(xTaskMelsecHandle);
        xTaskMelsecHandle = NULL;
        LOG_INFO(logger, "STOPPED THE MELSEC TASK");
    }

    bool HasMelsecTask()
    {
        if (xTaskMelsecHandle == NULL)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}