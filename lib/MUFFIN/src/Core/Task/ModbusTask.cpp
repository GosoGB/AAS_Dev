/**
 * @file ModbusTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Modbus 프로토콜로 데이터를 수집하는 태스크를 정의합니다.
 * 
 * @date 2024-10-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Protocol/Modbus/ModbusRTU.h"



namespace muffin {

    TaskHandle_t xTaskModbusRtuHandle = NULL;
    constexpr uint16_t KILLOBYTE = 1024;
    constexpr uint16_t SECOND_IN_MILLIS = 1000;



    void implModbusRtuTask(void* pvParameters)
    {
        ModbusRTU& modbusRTU = ModbusRTU::GetInstance();

        while (true)
        {
            Status ret = modbusRTU.Poll();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO POLL DATA: %s", ret.c_str());
            }
        
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
    }


    void StartModbusTask()
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
           10 * KILLOBYTE,		    // Stack memory size to allocate
            NULL,			        // Task parameters to be passed to the function
            0,				        // Task Priority for scheduling
            &xTaskModbusRtuHandle,       // The identifier of this task for machines
            0				        // Index of MCU core where the function to run
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


    void StopModbusTask()
    {
        if (xTaskModbusRtuHandle == NULL)
        {
            LOG_WARNING(logger, "NO MODBUS RTU TASK TO STOP!");
            return;
        }
        
        vTaskDelete(xTaskModbusRtuHandle);
        xTaskModbusRtuHandle = NULL;
    }
}