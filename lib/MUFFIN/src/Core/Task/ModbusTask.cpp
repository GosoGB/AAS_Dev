/**
 * @file ModbusTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Modbus 프로토콜로 데이터를 수집하는 태스크를 정의합니다.
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

#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusMutex.h"

#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"


namespace muffin {

    std::vector<ModbusTCP> ModbusTcpVector;
    std::vector<ModbusTCP> ModbusTcpVectorDynamic;
    std::vector<ModbusRTU> ModbusRtuVector;


    TaskHandle_t xTaskModbusRtuHandle = NULL;
    TaskHandle_t xTaskModbusTcpHandle = NULL;


    void implModbusRtuTask(void* pvParameter)
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
        
                LOG_DEBUG(logger, "[ModbusRtuTask} Stack Remaind: %u Bytes", RemainedStackSize);

                deviceStatus.SetTaskRemainedStack(task_name_e::MODBUS_RTU_TASK, RemainedStackSize);
            }

            if (g_DaqTaskSetFlag.test((static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK)) == true))
            {
                continue;
            }
            
            for(auto& modbusRTU : ModbusRtuVector)
            {

            #if defined(MODLINK_L) || defined(ML10) || defined(MT11)
                Status ret = modbusRTU.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }
            #else
                Status ret = modbusRTU.PollTemp();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }
            #endif
            }

            g_DaqTaskSetFlag.set(static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK));
            vTaskDelay(s_PollingIntervalInMillis / portTICK_PERIOD_MS);
        }
    }

    void StartModbusRtuTask()
    {
        if (xTaskModbusRtuHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }

        /**
         * @todo 스택 오버플로우를 방지하기 위해 태스크의 메모리 사용량에 따라
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implModbusRtuTask,      // Function to be run inside of the task
            "implModbusRtuTask",    // The identifier of this task for men
            5 * KILLOBYTE,          // Stack memory size to allocate
            NULL,			        // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskModbusRtuHandle,  // The identifier of this task for machines
            0				        // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Modbus RTU task has been started");
            g_DaqTaskEnableFlag.set(static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK));
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

    void StopModbusRtuTask()
    {
        if (xTaskModbusRtuHandle == NULL)
        {
            LOG_WARNING(logger, "NO MODBUS RTU TASK TO STOP!");
            return;
        }
        g_DaqTaskEnableFlag.reset(static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK));
        vTaskDelete(xTaskModbusRtuHandle);
        xTaskModbusRtuHandle = NULL;
        LOG_INFO(logger, "STOPPED THE MODBUS RTU TASK");
    }

    bool HasModbusRtuTask()
    {
        if (xTaskModbusRtuHandle == NULL)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    void implModbusTcpTask(void* pvParameter)
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

                LOG_DEBUG(logger, "[ModbusTcpTask] Stack Remaind: %u Bytes", RemainedStackSize);
                
                deviceStatus.SetTaskRemainedStack(task_name_e::MODBUS_TCP_TASK, RemainedStackSize);
            }

            if (g_DaqTaskSetFlag.test((static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK)) == true))
            {
                continue;
            }
            
            for(auto& modbusTCP : ModbusTcpVector)
            {
                if (xSemaphoreTake(xSemaphoreModbusTCP, 2000)  != pdTRUE)
                {
                    LOG_WARNING(logger, "[MODBUS TCP] THE READ MODULE IS BUSY. TRY LATER.");
                    continue;
                }

                if (!modbusTCP.mModbusTCPClient->connected()) 
                {
                    if (modbusTCP.mModbusTCPClient->begin(modbusTCP.GetServerIP(), modbusTCP.GetServerPort()) != 1) 
                    {
                        LOG_ERROR(logger,"Modbus TCP Client failed to connect!, serverIP : %s, serverPort: %d", modbusTCP.GetServerIP().toString().c_str(), modbusTCP.GetServerPort());
                        modbusTCP.SetTimeoutError();
                        xSemaphoreGive(xSemaphoreModbusTCP);
                        continue;
                    } 
                    else
                    {
                        LOG_DEBUG(logger,"Modbus TCP Client connected");
                    }
                }

                Status ret = modbusTCP.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }

                xSemaphoreGive(xSemaphoreModbusTCP);

            }
        #if defined(MT11)
            for(auto& modbusTCP : ModbusTcpVectorDynamic)
            {
                if (xSemaphoreTake(xSemaphoreModbusTCP, 2000)  != pdTRUE)
                {
                    LOG_WARNING(logger, "[MODBUS TCP] THE READ MODULE IS BUSY. TRY LATER.");
                    continue;
                }

                if (modbusTCP.mModbusTCPClient->begin(modbusTCP.GetServerIP(), modbusTCP.GetServerPort()) != 1) 
                {
                    LOG_ERROR(logger,"Modbus TCP Client failed to connect!, serverIP : %s, serverPort: %d", modbusTCP.GetServerIP().toString().c_str(), modbusTCP.GetServerPort());
                    modbusTCP.SetTimeoutError();
                    
                    xSemaphoreGive(xSemaphoreModbusTCP);
                    continue;
                }
                else
                {
                    LOG_DEBUG(logger,"Modbus TCP Client connected");
                }

                Status ret = modbusTCP.Poll();
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
                }

                modbusTCP.mModbusTCPClient->end();
                
                xSemaphoreGive(xSemaphoreModbusTCP);
            }
        #endif
            g_DaqTaskSetFlag.set(static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK));
            vTaskDelay(s_PollingIntervalInMillis / portTICK_PERIOD_MS);   
        }
    }


    void StartModbusTcpTask()
    {
        if (xTaskModbusTcpHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        xSemaphoreModbusTCP = xSemaphoreCreateMutex();
        if (xSemaphoreModbusTCP == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE MODBUS TCP SEMAPHORE");
            return;
        }

        /**
         * @todo 스택 오버플로우를 방지하기 위해 태스크의 메모리 사용량에 따라
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implModbusTcpTask,      // Function to be run inside of the task
            "implModbusTcpTask",    // The identifier of this task for men
            5 * KILLOBYTE,		    // Stack memory size to allocate
            NULL,       // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskModbusTcpHandle,       // The identifier of this task for machines
            1				        // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Modbus TCP task has been started");
            g_DaqTaskEnableFlag.set(static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK));
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


    void StopModbusTcpTask()
    {
        if (xTaskModbusTcpHandle == NULL)
        {
            LOG_WARNING(logger, "NO MODBUS TCP TASK TO STOP!");
            return;
        }
        g_DaqTaskEnableFlag.reset(static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK));
        vTaskDelete(xTaskModbusTcpHandle);
        xTaskModbusTcpHandle = NULL;
        LOG_INFO(logger, "STOPPED THE MODBUS TCP TASK");
    }

    bool HasModbusTcpTask()
    {
        if (xTaskModbusTcpHandle == NULL)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}