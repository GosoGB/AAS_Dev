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

    bitset<static_cast<uint8_t>(3)> g_DaqTaskEnableFlag;
    bitset<static_cast<uint8_t>(3)> g_DaqTaskSetFlag;

    bool WaitForFlagWithTimeout(set_task_flag_e task)
    {
        if (!g_DaqTaskEnableFlag.test(static_cast<uint8_t>(task))) 
        {
            return true;
        }

        if (g_DaqTaskSetFlag.test(static_cast<uint8_t>(task)))
        {
            return true;
        }
        
        return false;
    }
    

    void MSGTask(void* pvParameter)
    {
        const size_t batchSize = 4 * 1024;
        char* batchPayload = static_cast<char*>(malloc(4 * 1024));
        if (batchPayload == nullptr) 
        {
            LOG_ERROR(logger, "Failed to allocate batchPayload buffer");
            return;
        }
        memset(batchPayload, 0, batchSize);

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        std::vector<im::Node*> cyclicalNodeVector = nodeStore.GetCyclicalNode();
        std::vector<im::Node*> eventNodeVector = nodeStore.GetEventNode();

        static uint32_t currentTimestamp = 0;
        const uint32_t intervalMillis = s_PublishIntervalInSeconds * SECOND_IN_MILLIS;
        uint32_t statusReportMillis = millis(); 

        bool initFlag = true;
        while (true)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);

            bool isSuccessPolling = true;
            uint64_t sourceTimestamp = GetTimestampInMillis();
        #if defined(DEBUG)
            if ((millis() - statusReportMillis) > (30 * SECOND_IN_MILLIS))
        #else
            if ((millis() - statusReportMillis) > (3550 * SECOND_IN_MILLIS))
        #endif
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
                LOG_INFO(logger, "[MSGTask] Stack Remaind: %u Bytes", RemainedStackSize);
                
                deviceStatus.SetTaskRemainedStack(task_name_e::PUBLISH_MSG_TASK, RemainedStackSize);
            }
            
            if (WaitForFlagWithTimeout(set_task_flag_e::MODBUS_RTU_TASK) == false)
            {
                isSuccessPolling = false;
            }
            
            if (WaitForFlagWithTimeout(set_task_flag_e::MODBUS_TCP_TASK) == false)
            {
                isSuccessPolling = false;
            }

            if (WaitForFlagWithTimeout(set_task_flag_e::MELSEC_TASK) == false)
            {
                isSuccessPolling = false;
            }

            if (initFlag == true)
            {
                if(isSuccessPolling)
                {
                   initFlag = false; 
                   g_DaqTaskSetFlag.reset();
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (isSuccessPolling)
                {
                    g_DaqTaskSetFlag.reset();
                }
            }
            
            std::vector<json_datum_t> nodeVector;
            nodeVector.reserve(cyclicalNodeVector.size() + eventNodeVector.size());
            
            if (isSuccessPolling)
            {
                for (auto& node : eventNodeVector)
                {
                    if (node->VariableNode.mHasNewEvent == false    ||  
                        node->GetTopic() == mqtt::topic_e::ALARM    || 
                        node->GetTopic() == mqtt::topic_e::ERROR
                    )
                    {   
                        continue;
                    }
                    
                    std::pair<bool, json_datum_t> ret;
                    ret = node->VariableNode.CreateDaqStruct();
                    if (ret.first != true)
                    {
                        ret.second.Value = "MFM_NULL";
                        ret.second.SourceTimestamp = GetTimestampInMillis();
                    }

                    if (node->GetTopic() == mqtt::topic_e::DAQ_PARAM)
                    {
                        JSON json;
                        const size_t size = UINT8_MAX;
                        char payload[size] = {'\0'};
                        json.Serialize(ret.second, size, payload);
                        mqtt::Message message(ret.second.Topic, payload);
                        mqtt::cdo.Store(message);
                    }
                    else
                    {
                        nodeVector.emplace_back(ret.second);
                    }
                }
            }  
            
            uint32_t now = millis();
            if (now >= currentTimestamp)
            {
                currentTimestamp = (currentTimestamp == 0) ? now + intervalMillis : currentTimestamp + intervalMillis;

                for (auto& node : cyclicalNodeVector)
                {
                    std::pair<bool, json_datum_t> ret;
                    ret = node->VariableNode.CreateDaqStruct();
                    if (ret.first != true)
                    {
                        ret.second.Value = "MFM_NULL";
                    }
                    nodeVector.emplace_back(ret.second);
                }
            }

            if (nodeVector.empty())
            {
                continue;
            }

            JSON json;
            const size_t TESTSIZE = 200;
            LOG_INFO(logger,"nodeVector.size() : %u",nodeVector.size());
            for (size_t i = 0; i < nodeVector.size(); i += TESTSIZE) 
            {
                memset(batchPayload, 0, batchSize);
                size_t end = std::min(i + TESTSIZE, nodeVector.size());
                std::vector<json_datum_t> batch(nodeVector.begin() + i, nodeVector.begin() + end);
                json.Serialize(batch, batchSize, sourceTimestamp, batchPayload);
                mqtt::Message message(mqtt::topic_e::DAQ_INPUT, batchPayload);
                mqtt::cdo.Store(message);
            }
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
    #if defined(MT11)
            8*KILLOBYTE,			       // Stack memory size to allocate
    #else
            4*KILLOBYTE,			       // Stack memory size to allocate
    #endif
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