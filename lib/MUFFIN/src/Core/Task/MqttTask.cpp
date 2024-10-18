/**
 * @file MqttTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 상태 모니터링 및 관리와 수신한 메시지를 처리하는 태스크를 정의합니다.
 * 
 * @date 2024-10-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Core/Core.h"
#include "MqttTask.h"
#include "Protocol/MQTT/CIA.h"



namespace muffin {

    TaskHandle_t xTaskMqttHandle;

    void StartTaskMQTT()
    {
        if (xTaskMqttHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implMqttTask,      // Function to be run inside of the task
            "implMqttTask",    // The identifier of this task for men
            10240,			   // Stack memory size to allocate
            NULL,			   // Task parameters to be passed to the function
            0,				   // Task Priority for scheduling
            &xTaskMqttHandle,  // The identifier of this task for machines
            0				   // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The MQTT task has been started");
            break;

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            break;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            break;

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            break;
        }
    }

    void implMqttTask(void* pvParameter)
    {
        mqtt::CIA& cia = mqtt::CIA::GetInstance();

        while (true)
        {
            if (cia.Count() == 0)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            const auto receivedMessage = cia.Retrieve();
            const Status status = receivedMessage.first;
            if (status.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE MESSAGE FROM CIA: %s", status.c_str());
            }

            Core& core = Core::GetInstance();
            core.HandleMqttMessage(receivedMessage.second);
        }
    }
}