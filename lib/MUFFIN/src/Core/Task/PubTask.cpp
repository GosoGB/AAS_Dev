/**
 * @file PubTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
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
#include "PubTask.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/Node/Node.h"
#include "IM/Node/NodeStore.h"
#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"


namespace muffin {

    TaskHandle_t xTaskMonitorHandle = NULL;

    bitset<static_cast<uint8_t>(3)> s_DaqTaskEnableFlag;
    bitset<static_cast<uint8_t>(3)> s_DaqTaskSetFlag;

    void MSGTask(void* pvParameter)
    {
        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        std::vector<im::Node*> cyclicalNodeVector = nodeStore.GetCyclicalNode();
        std::vector<im::Node*> eventNodeVector = nodeStore.GetEventNode();

        uint32_t statusReportMillis = millis(); 
        time_t currentTimestamp = 0;

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
                LOG_DEBUG(logger, "[MSGTask] Stack Remaind: %u Bytes", RemainedStackSize);
                
                deviceStatus.SetTaskRemainedStack(task_name_e::PUBLISH_MSG_TASK, RemainedStackSize);
            }
            
            uint32_t lastReceiveTime;
            if (s_DaqTaskEnableFlag.test(static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK)))
            {
                lastReceiveTime = millis();

                while (s_DaqTaskSetFlag.test(static_cast<uint8_t>(set_task_flag_e::MODBUS_RTU_TASK)) == false)
                {
                    if (millis() - lastReceiveTime > (2 * SECOND_IN_MILLIS)) 
                    {
                        LOG_ERROR(logger,"TIMEOUT: MODBUS_RTU_TASK");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                LOG_INFO(logger,"POLLING MODBUS_RTU_TASK");
            }
            
            if (s_DaqTaskEnableFlag.test(static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK)))
            {
                lastReceiveTime = millis();

                while (s_DaqTaskSetFlag.test(static_cast<uint8_t>(set_task_flag_e::MODBUS_TCP_TASK)) == false)
                {
                    if (millis() - lastReceiveTime > (2 * SECOND_IN_MILLIS)) 
                    {
                        LOG_ERROR(logger,"TIMEOUT: MODBUS_TCP_TASK");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                LOG_INFO(logger,"POLLING MODBUS_TCP_TASK");
            }

            if (s_DaqTaskEnableFlag.test(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK)))
            {
                lastReceiveTime = millis();

                while (s_DaqTaskSetFlag.test(static_cast<uint8_t>(set_task_flag_e::MELSEC_TASK)) == false)
                {
                    if (millis() - lastReceiveTime > (2 * SECOND_IN_MILLIS)) 
                    {
                        LOG_ERROR(logger,"TIMEOUT: MELSEC_TASK");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                LOG_INFO(logger,"POLLING MELSEC_TASK");
            }

            s_DaqTaskSetFlag.reset();

            std::vector<json_datum_t> nodeVector;
            nodeVector.reserve(cyclicalNodeVector.size() + eventNodeVector.size());
            
            for (auto& node : eventNodeVector)
            {
                if (node->VariableNode.mHasNewEvent == false)
                {
                    continue;
                }
                
                std::pair<bool, json_datum_t> ret;
                ret = node->VariableNode.CreateDaqStruct();
                if (ret.first != true)
                {
                    ret.second.Value = "MFM_NULL_DATA";
                }
                nodeVector.emplace_back(ret.second);
            }
            
            if (GetTimestamp() - currentTimestamp > s_PublishIntervalInSeconds)
            {
                currentTimestamp = GetTimestamp();
                for (auto& node : cyclicalNodeVector)
                {
                    std::pair<bool, json_datum_t> ret;
                    ret = node->VariableNode.CreateDaqStruct();
                    if (ret.first != true)
                    {
                        ret.second.Value = "MFM_NULL_DATA";
                    }
                    nodeVector.emplace_back(ret.second);
                }
            }

            if (nodeVector.empty())
            {
                continue;
            }
            
            uint64_t SourceTimestamp = GetTimestampInMillis();
            JSON json;
            const size_t size = 4 * 1024;
            char payload[size] = {'\0'};
            json.Serialize(nodeVector, size, SourceTimestamp, payload);
            mqtt::Message message(mqtt::topic_e::DAQ_INPUT, payload);
            mqtt::cdo.Store(message);

            vTaskDelay(100 / portTICK_PERIOD_MS); 
        }
    }

    void StartTaskMSG()
    {
        if (xTaskMonitorHandle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해서 MQTT 메시지 크기에 따라서
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            MSGTask,      // Function to be run inside of the task
            "MSGTask",    // The identifier of this task for men
            8*KILLOBYTE,			       // Stack memory size to allocate
            NULL,      // Task parameters to be passed to the function
            0,				       // Task Priority for scheduling
            &xTaskMonitorHandle,   // The identifier of this task for machines
            0				       // Index of MCU core where the function to run
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

    void StopMSGTask()
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