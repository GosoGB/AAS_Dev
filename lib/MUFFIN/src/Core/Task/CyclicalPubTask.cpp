/**
 * @file CyclicalPubTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-10-29
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <esp32-hal.h>

#include "DataFormat/JSON/JSON.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Core/Core.h"
#include "CyclicalPubTask.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/Node/Node.h"
#include "IM/Node/NodeStore.h"


namespace muffin {

    TaskHandle_t xTaskMonitorHandle = NULL;

    void StartTaskCyclicalsMSG(uint16_t pollingInterval)
    {
        if (xTaskMonitorHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        std::vector<im::Node*> cyclicalNodeVector = nodeStore.GetCyclicalNode();
        
        if (cyclicalNodeVector.empty())
        {
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해서 MQTT 메시지 크기에 따라서
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            cyclicalsMSGTask,      // Function to be run inside of the task
            "cyclicalsMSGTask",    // The identifier of this task for men
            10240,			   // Stack memory size to allocate
            &pollingInterval,			   // Task parameters to be passed to the function
            0,				   // Task Priority for scheduling
            &xTaskMonitorHandle,  // The identifier of this task for machines
            0				   // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Monitor task has been started");
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

    void cyclicalsMSGTask(void* pvParameter)
    {
        uint16_t publishInterval = *(uint16_t*) pvParameter;
        time_t currentTimestamp = GetTimestamp();
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        while (true)
        {
            if (GetTimestamp() - currentTimestamp < publishInterval)
            {
                vTaskDelay(100 / portTICK_PERIOD_MS); 
                continue;
            }

            currentTimestamp = GetTimestamp();

            LOG_DEBUG(logger,"1분 경과 %lu",currentTimestamp);

            im::NodeStore& nodeStore = im::NodeStore::GetInstance();
            std::vector<im::Node*> cyclicalNodeVector = nodeStore.GetCyclicalNode();
            
            for (auto& node : cyclicalNodeVector)
            {
                std::pair<bool, daq_struct_t> ret;
                ret = node->VariableNode.CreateDaqStruct();

                if (ret.first == true)
                {
                    JSON json;
                    const std::string payload = json.Serialize(ret.second);
                    mqtt::Message message(ret.second.Topic, payload);

                    mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                    cdo.Store(message);
                }
            }
            
            
            vTaskDelay(100 / portTICK_PERIOD_MS); 
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: Modbus] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }

    void StopCyclicalsMSGTask()
    {
        if (xTaskMonitorHandle == NULL)
        {
            LOG_WARNING(logger, "NO MODBUS RTU TASK TO STOP!");
            return;
        }
        
        vTaskDelete(xTaskMonitorHandle);
        xTaskMonitorHandle = NULL;
    }

}