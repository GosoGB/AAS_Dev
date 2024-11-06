/**
 * @file DeprecableOperationTime.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 가동시간 정보를 집계하고 매분 서버로 전송하는 클래스를 정의합니다.
 * 
 * @date 2024-10-29
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DeprecableOperationTime.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "Protocol/MQTT/CDO.h"



namespace muffin {

    OperationTime& OperationTime::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) OperationTime();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR OPERATION TIME");
            }
        }

        return *mInstance;
    }
    
    OperationTime::OperationTime()
        : mCriterion(std::make_pair(Status(Status::Code::BAD), 0))
        , mOperator(std::make_pair(Status(Status::Code::BAD), jarvis::cmp_op_e::EQUAL))
        , xHandle(NULL)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    OperationTime::~OperationTime()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void OperationTime::Config(jarvis::config::OperationTime* cin)
    {
        mNodeId = cin->GetNodeID().second;
        mType = cin->GetType().second;
        if (mType != jarvis::op_time_type_e::FROM_MODLINK)
        {
            return;
        }
        ASSERT((mType == jarvis::op_time_type_e::FROM_MODLINK), "UNSUPPORTED AGGREGATE TYPE FOR OPERATION TIME");
        
        mCriterion  = cin->GetCriterion();
        mOperator   = cin->GetOperator();

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        for (auto& node : nodeStore)
        {
            if (node.first == mNodeId)
            {
                mVectorNodeReference.emplace_back(node);
            #if defined(DEBUG)
                LOG_DEBUG(logger, "Node Reference: %s", mVectorNodeReference.front().get().first.c_str());
            #endif
            }
        }
    }
    
    void OperationTime::Clear()
    {
        mNodeId.clear();
        mCriterion.first = Status::Code::BAD;
        mOperator.first  = Status::Code::BAD;
        mPublishTimer.LastTime = 0;
        mPublishTimer.NextTime = 0;
        mVectorNodeReference.clear();

        LOG_INFO(logger, "Operation time configurations and data have been cleared");
    }

    void OperationTime::StartTask()
    {
        if (xHandle != NULL)
        {
            return;
        }
        
        /**
         * @todo 스택 오버플로우를 방지하기 위해 태스크의 메모리 사용량에 따라
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            wrapImplTask,    // Function to be run inside of the task
            "OpTimeTask",    // The identifier of this task for men
            8 * 1024,	     // Stack memory size to allocate
            this,	         // Task parameters to be passed to the function
            0,		         // Task Priority for scheduling
            &xHandle,        // The identifier of this task for machines
            1	             // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The OperationTime task has been started");
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

    void OperationTime::StopTask()
    {
        if (xHandle == NULL)
        {
            LOG_WARNING(logger, "NO OPERATION TIME TASK TO STOP!");
            return;
        }
        
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    void OperationTime::wrapImplTask(void* pvParams)
    {
        static_cast<OperationTime*>(pvParams)->implTask();
    }

    void OperationTime::implTask()
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        mPublishTimer.LastTime = GetTimestamp();
        mPublishTimer.NextTime = CalculateTimestampNextMinuteStarts(mPublishTimer.LastTime);
        
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        time_t processingTime = 0;
 
        while (true)
        {
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: OperationTime] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(xHandle));
                checkRemainedStackMillis = millis();
            }
        #endif

            for (auto& nodeReference : mVectorNodeReference)
            {
                const size_t dataStoredCount = nodeReference.get().second.VariableNode.RetrieveCount();
                if (dataStoredCount == 0)
                {
                    break;
                }
                ASSERT((dataStoredCount != 0), "THERE MUST BE DATA COLLECTED TO AGGREGATE PRODUCTION INFO");

                auto& reference = nodeReference.get();
                const im::var_data_t datum = reference.second.VariableNode.RetrieveData();
                if (datum.StatusCode != Status::Code::GOOD)
                {
                    break;
                }

                bool isStatusProcessing = false;
                bool currentErrorStatus = alarmMonitor.HasError();
                updateOperationTime(datum, reference.second, &isStatusProcessing);
                
                if (isStatusProcessing == true)
                {
                    if (currentErrorStatus == false)
                    {
                        if (mStauts != jarvis::op_status_e::PROCESSING)
                        {
                            mStauts = jarvis::op_status_e::PROCESSING;
                            publishOperationStauts();
                        }
                        
                        ++processingTime;
                    }
                }
                else
                {
                    if (mStauts != jarvis::op_status_e::IDLE)
                    {
                        mStauts = jarvis::op_status_e::IDLE;
                        publishOperationStauts();
                    }
                }

                if (GetTimestamp() < mPublishTimer.NextTime)
                {
                    break;
                }

                publishInfo(processingTime);

                mPublishTimer.LastTime = mPublishTimer.NextTime;
                mPublishTimer.NextTime = CalculateTimestampNextMinuteStarts(mPublishTimer.LastTime);
                processingTime = 0;
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void OperationTime::updateOperationTime(const im::var_data_t& datum, im::Node& node, bool* isStatusProcessing)
    {
        switch (mOperator.second)
        {
        case jarvis::cmp_op_e::EQUAL:
            *isStatusProcessing = strategyEqual(datum, node);
            break;
        case jarvis::cmp_op_e::GREATER_EQUAL:
            *isStatusProcessing = strategyGreaterOrEqual(datum, node);
            break;
        case jarvis::cmp_op_e::GREATER_THAN:
            *isStatusProcessing = strategyGreaterThan(datum, node);
            break;
        case jarvis::cmp_op_e::LESS_EQUAL:
            *isStatusProcessing = strategyLessOrEqual(datum, node);
            break;
        case jarvis::cmp_op_e::LESS_THAN:
            *isStatusProcessing = strategyLessThan(datum, node);
            break;
        default:
            break;
        }
    }

    bool OperationTime::strategyEqual(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Boolean);

        case jarvis::dt_e::INT8:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int8);

        case jarvis::dt_e::UINT8:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt8);

        case jarvis::dt_e::INT16:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int16);

        case jarvis::dt_e::UINT16:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt16);

        case jarvis::dt_e::INT32:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int32);

        case jarvis::dt_e::UINT32:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt32);

        case jarvis::dt_e::INT64:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int64);

        case jarvis::dt_e::UINT64:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt64);

        case jarvis::dt_e::FLOAT32:
            return mCriterion.second == datum.Value.Float32;

        case jarvis::dt_e::FLOAT64:
            return mCriterion.second == datum.Value.Float64;

        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.VariableNode.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        return mCriterion.second == static_cast<int32_t>(pair.first);
                    }
                }
                return false;
            }
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyGreaterOrEqual(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Boolean);

        case jarvis::dt_e::INT8:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int8);

        case jarvis::dt_e::UINT8:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt8);

        case jarvis::dt_e::INT16:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int16);

        case jarvis::dt_e::UINT16:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt16);

        case jarvis::dt_e::INT32:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int32);

        case jarvis::dt_e::UINT32:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt32);

        case jarvis::dt_e::INT64:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int64);

        case jarvis::dt_e::UINT64:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt64);

        case jarvis::dt_e::FLOAT32:
            return mCriterion.second >= datum.Value.Float32;

        case jarvis::dt_e::FLOAT64:
            return mCriterion.second >= datum.Value.Float64;

        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.VariableNode.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        return mCriterion.second >= static_cast<int32_t>(pair.first);
                    }
                }
                return false;
            }
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyGreaterThan(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Boolean);

        case jarvis::dt_e::INT8:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int8);

        case jarvis::dt_e::UINT8:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt8);

        case jarvis::dt_e::INT16:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int16);

        case jarvis::dt_e::UINT16:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt16);

        case jarvis::dt_e::INT32:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int32);

        case jarvis::dt_e::UINT32:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt32);

        case jarvis::dt_e::INT64:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int64);

        case jarvis::dt_e::UINT64:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt64);

        case jarvis::dt_e::FLOAT32:
            return mCriterion.second > datum.Value.Float32;

        case jarvis::dt_e::FLOAT64:
            return mCriterion.second > datum.Value.Float64;

        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.VariableNode.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        return mCriterion.second > static_cast<int32_t>(pair.first);
                    }
                }
                return false;
            }
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyLessOrEqual(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Boolean);

        case jarvis::dt_e::INT8:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int8);

        case jarvis::dt_e::UINT8:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt8);

        case jarvis::dt_e::INT16:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int16);

        case jarvis::dt_e::UINT16:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt16);

        case jarvis::dt_e::INT32:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int32);

        case jarvis::dt_e::UINT32:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt32);

        case jarvis::dt_e::INT64:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int64);

        case jarvis::dt_e::UINT64:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt64);

        case jarvis::dt_e::FLOAT32:
            return mCriterion.second <= datum.Value.Float32;

        case jarvis::dt_e::FLOAT64:
            return mCriterion.second <= datum.Value.Float64;

        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.VariableNode.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        return mCriterion.second <= static_cast<int32_t>(pair.first);
                    }
                }
                return false;
            }
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyLessThan(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Boolean);

        case jarvis::dt_e::INT8:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int8);

        case jarvis::dt_e::UINT8:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt8);

        case jarvis::dt_e::INT16:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int16);

        case jarvis::dt_e::UINT16:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt16);

        case jarvis::dt_e::INT32:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int32);

        case jarvis::dt_e::UINT32:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt32);

        case jarvis::dt_e::INT64:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int64);

        case jarvis::dt_e::UINT64:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt64);

        case jarvis::dt_e::FLOAT32:
            return mCriterion.second < datum.Value.Float32;

        case jarvis::dt_e::FLOAT64:
            return mCriterion.second < datum.Value.Float64;

        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.VariableNode.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        return mCriterion.second < static_cast<int32_t>(pair.first);
                    }
                }
                return false;
            }
            
        default:
            return false;
        }
    }
    
    void OperationTime::publishInfo(const time_t processingTime)
    {
        progix_struct_t production;

        production.Value = std::to_string(processingTime);
        production.Topic = mqtt::topic_e::UPTIME;
        production.SourceTimestamp = TimestampToExactHourKST();

        JSON json;
        const std::string payload = json.Serialize(production);
        mqtt::Message message(mqtt::topic_e::UPTIME, payload);

        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        cdo.Store(message);

    }

    void OperationTime::publishOperationStauts()
    {
        operation_struct_t status;

        status.SourceTimestamp = GetTimestampInMillis();
        status.Status = mStauts == jarvis::op_status_e::PROCESSING ? "processing" : "idle";
        status.Topic = mqtt::topic_e::OPERATION;

        JSON json;
        const std::string payload = json.Serialize(status);
        mqtt::Message message(mqtt::topic_e::OPERATION, payload);

        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        cdo.Store(message);

    }



    OperationTime* OperationTime::mInstance = nullptr;
}