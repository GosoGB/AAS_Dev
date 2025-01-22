/**
 * @file MqttTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 상태 모니터링 및 관리와 수신한 메시지를 처리하는 태스크를 정의합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
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
#include "Protocol/MQTT/CDO.h"
#include "Protocol/MQTT/IMQTT.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"

namespace muffin {

    TaskHandle_t xTaskMqttHandle = NULL;

    void StartTaskMQTT()
    {
        if (xTaskMqttHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해서 MQTT 메시지 크기에 따라서
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implMqttTask,      // Function to be run inside of the task
            "implMqttTask",    // The identifier of this task for men
            5 * 1024,	       // Stack memory size to allocate
            NULL,			   // Task parameters to be passed to the function
            0,				   // Task Priority for scheduling
            &xTaskMqttHandle,  // The identifier of this task for machines
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

        // mqtt::IMQTT* serviceNetwork; 인자로 추가
        taskCreationResult = xTaskCreatePinnedToCore(
            publishMqttTask,      // Function to be run inside of the task
            "publishMqttTask",    // The identifier of this task for men
            5 * 1024,			      // Stack memory size to allocate
            NULL,			      // Task parameters to be passed to the function
            0,				      // Task Priority for scheduling
            &xTaskMqttHandle,     // The identifier of this task for machines
            0				      // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Publish MQTT task has been started");
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

    void implMqttTask(void* pvParameter)
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 5 * 1000;
    #endif
    
        while (true)
        {
            if (mqtt::cia.Count() == 0)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            const auto receivedMessage = mqtt::cia.Retrieve();
            const Status status = receivedMessage.first;
            if (status.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE MESSAGE FROM CIA: %s", status.c_str());
                continue;
            }

            core.RouteMqttMessage(receivedMessage.second);

        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: CatM1] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }

    void publishMqttTask(void* pvParameter)
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 5 * 1000;
    #endif
    
        /**
         * @todo 이렇게 바꿀거임
         * 
         */
        // mqtt::IMQTT* serviceNetwork;
        // INetwork* nic = serviceNetwork->RetrieveNIC();
        

        INetwork* nic;
        mqtt::IMQTT* serviceNetwork;
        
        mqtt::LwipMQTT& lwipMqtt = mqtt::LwipMQTT::GetInstance();
        nic = lwipMqtt.RetrieveNIC();
        serviceNetwork = static_cast<mqtt::IMQTT*>(&lwipMqtt);

        const uint8_t MAX_RETRY_COUNT = 5;
        while (true)
        {
            if (mqtt::cdo.Count() == 0)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            const auto pubMessage = mqtt::cdo.Peek();
            const Status status = pubMessage.first;

            if (status.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO PEEK MESSAGE FROM CDO: %s ", status.c_str());
                vTaskDelay(500 / portTICK_PERIOD_MS);
                continue;
            }

            std::pair<Status, size_t> mutex = nic->TakeMutex();
            if (mutex.first != Status::Code::GOOD)
            {
                vTaskDelay(500 / portTICK_PERIOD_MS);
                continue;
            }
            
            for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
            {
                Status ret = serviceNetwork->Publish(mutex.second, pubMessage.second);
                if (ret == Status::Code::GOOD)
                {
                    mqtt::cdo.Retrieve();
                    break;
                }
                else if ((i + 1) == MAX_RETRY_COUNT)
                {
                    /**
                     * @todo 메시지 전송에 실패했을 때 나중에 다시 전송을 시도할 수 있도록 기능을 보완해야 합니다.
                     */
                    LOG_ERROR(logger, "FAILED TO PUBLISH MESSAGE: %s", ret.c_str());
                    /**
                     * @todo 메시지 전송에 실패한 경우에 return 해서 빠져나갈지 아니면 설정 정보를
                     *        저장한 다음 설정 정보를 적용할지 여부를 결정해야 합니다.
                     */
                    ASSERT(false, "IMPLEMENTATION ERROR: NEED TO DECIDE BETWEEN EARLY EXIT AND APPLYING THE CONFIG");
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "[TRIAL: #%u] PUBLISH WAS UNSUCCESSFUL: %s", i, ret.c_str());
                }
            }
            nic->ReleaseMutex();

        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: CatM1] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }
}