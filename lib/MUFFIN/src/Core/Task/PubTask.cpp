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
#include "esp_heap_caps.h"
#include <esp32-hal.h>

#include "DataFormat/JSON/JSON.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Assert.hpp"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Common/PSRAM.hpp"
#include "Core/Core.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "PubTask.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/Node/Node.h"
#include "IM/Node/NodeStore.h"
#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"


namespace muffin {

    TaskHandle_t xTaskMonitorHandle = NULL;

    bitset<static_cast<uint8_t>(4)> g_DaqTaskEnableFlag;
    bitset<static_cast<uint8_t>(4)> g_DaqTaskSetFlag;

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
    #if defined(MT11)
        psram::map<uint16_t, psram::vector<std::string>> IntervalCustomMap;

        if (jvs::config::operation.GetIntervalServerCustom().first == Status::Code::GOOD)
        {
            IntervalCustomMap = jvs::config::operation.GetIntervalServerCustom().second;
        }
        char* batchPayload = static_cast<char*>(
            heap_caps_malloc(batchSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
        );
    #else
        char* batchPayload = static_cast<char*>(malloc(4 * 1024));

        std::map<uint16_t, std::vector<std::string>> IntervalCustomMap;

        if (jvs::config::operation.GetIntervalServerCustom().first == Status::Code::GOOD)
        {
            IntervalCustomMap = jvs::config::operation.GetIntervalServerCustom().second;
        }

    #endif
        if (batchPayload == nullptr) 
        {
            LOG_ERROR(logger, "Failed to allocate batchPayload buffer");
            return;
        }
        memset(batchPayload, 0, batchSize);
        
        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        uint16_t defaultInterval = jvs::config::operation.GetIntervalServer().second;

    #if defined(MT11)
        psram::map<uint16_t, psram::vector<im::Node*>> IntervalNodeMap = nodeStore.GetIntervalCustomNode(IntervalCustomMap, defaultInterval);
        psram::vector<im::Node*> eventNodeVector = nodeStore.GetEventNode();
    #else
        std::map<uint16_t, std::vector<im::Node*>> IntervalNodeMap = nodeStore.GetIntervalCustomNode(IntervalCustomMap, defaultInterval);        
        std::vector<im::Node*> eventNodeVector = nodeStore.GetEventNode();
    #endif

        for (const auto& pair : IntervalNodeMap) 
        {
            LOG_INFO(logger,"Interval: %u, Node Count: %u", pair.first, pair.second.size());
            for (const auto& node : pair.second) 
            {
                LOG_INFO(logger," NodeID: %s", node->GetNodeID());
            }
        }
        
        uint32_t statusReportMillis = millis(); 
        bool initFlag = true;
        std::map<uint16_t, uint64_t> TimeTrackerMap;
        
        bool isFirstIntervalLoop = true;
        uint64_t baseIntervalTimestamp = 0;

        while (true)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            // uint32_t StartMillis = millis();

            bool isSuccessPolling = true;
            uint64_t sourceTimestamp = GetTimestampInMillis();
            if ((millis() - statusReportMillis) > (590 * SECOND_IN_MILLIS))
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
                LOG_DEBUG(logger, "[MSGTask] Stack Remaind: %u Bytes", RemainedStackSize);
                
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

            if (WaitForFlagWithTimeout(set_task_flag_e::ETHERNET_IP_TASK) == false)
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

        #if defined(MT11)
            psram::vector<json_datum_t> nodeVector;
            psram::vector<json_datum_t> nodeArrayVector;
        #else
            std::vector<json_datum_t> nodeVector;
            std::vector<json_datum_t> nodeArrayVector;
        #endif
            nodeVector.reserve(nodeStore.GetCyclicalNode().size() + eventNodeVector.size());
            nodeArrayVector.reserve(nodeStore.GetArrayNodeCount());

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
                        if (node->IsArrayNode() == true)
                        {
                            nodeArrayVector.emplace_back(ret.second);
                        }
                        else
                        {
                            nodeVector.emplace_back(ret.second);
                        }
                        
                    }
                }
            }

            uint64_t now = GetTimestampInMillis();

            if (isFirstIntervalLoop) 
            {
                baseIntervalTimestamp = now;
                isFirstIntervalLoop = false;
            }

            for (const auto& pair : IntervalNodeMap) 
            {
                uint16_t interval = pair.first;
                const auto& nodeVec = pair.second;
                uint64_t intervalMillis = static_cast<uint64_t>(interval) * SECOND_IN_MILLIS;

                // 첫 루프에서는 모든 interval의 타임스탬프를 동일 기준으로 설정
                if (TimeTrackerMap.find(interval) == TimeTrackerMap.end()) 
                {
                    TimeTrackerMap[interval] = baseIntervalTimestamp;
                }

                if ((int64_t)(now - TimeTrackerMap[interval]) >= 0)
                {
                    TimeTrackerMap[interval] = baseIntervalTimestamp + (((now - baseIntervalTimestamp) / intervalMillis) + 1) * intervalMillis;

                    LOG_WARNING(logger, "Interval: %u, Node Count: %u", interval, nodeVec.size());
                    LOG_WARNING(logger, "TimeTrackerMap[%u]: %llu", interval, TimeTrackerMap[interval]);

                    for (auto& node : nodeVec) 
                    {
                        std::pair<bool, json_datum_t> ret = node->VariableNode.CreateDaqStruct();
                        if (ret.first != true) 
                        {
                            ret.second.Value = "MFM_NULL";
                        }

                        if (node->IsArrayNode() == true) 
                        {
                            nodeArrayVector.emplace_back(ret.second);
                        } 
                        else 
                        {
                            nodeVector.emplace_back(ret.second);
                        }
                    }
                }
            }

            if (!nodeVector.empty())
            {
                JSON json;
                const size_t TESTSIZE = 150;
                LOG_DEBUG(logger,"nodeVector.size() : %u",nodeVector.size());
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
            

            if (!nodeArrayVector.empty())
            {
                JSON json;
                const size_t MAX_ARRAY_TOTAL = 150;
                LOG_DEBUG(logger, "nodeArrayVector.size() : %u", nodeArrayVector.size());

                std::vector<json_datum_t> currentBatch;
                size_t currentArraySum = 0;

                for (const auto& node : nodeArrayVector)
                {
                    size_t arraySize = node.ArrayValue.size();

                    if (currentArraySum + arraySize > MAX_ARRAY_TOTAL)
                    {
                        // 배치 전송
                        memset(batchPayload, 0, batchSize);
                        json.Serialize(currentBatch, batchSize, sourceTimestamp, batchPayload);
                        mqtt::Message message(mqtt::topic_e::DAQ_INPUT, batchPayload);
                        mqtt::cdo.Store(message);

                        // 다음 배치를 위한 초기화
                        currentBatch.clear();
                        currentArraySum = 0;
                    }

                    currentBatch.emplace_back(node);
                    currentArraySum += arraySize;
                }

                // 남은 마지막 배치 처리
                if (!currentBatch.empty())
                {
                    memset(batchPayload, 0, batchSize);
                    json.Serialize(currentBatch, batchSize, sourceTimestamp, batchPayload);
                    mqtt::Message message(mqtt::topic_e::DAQ_INPUT, batchPayload);
                    mqtt::cdo.Store(message);
                }           
            }
            
            // LOG_DEBUG(logger, "[MSGTask] Loop Time: %lu ms", millis() - StartMillis);
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