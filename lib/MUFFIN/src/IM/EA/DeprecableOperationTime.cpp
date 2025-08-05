/**
 * @file DeprecableOperationTime.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 가동시간 정보를 집계하고 매분 서버로 전송하는 클래스를 정의합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DeprecableOperationTime.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/Custom/Device/DeviceStatus.h"
#include "IM/Custom/Constants.h"
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
        , mOperator(std::make_pair(Status(Status::Code::BAD), jvs::cmp_op_e::EQUAL))
        , xHandle(NULL)
    {
    }
    
    OperationTime::~OperationTime()
    {
    }

    void OperationTime::Config(jvs::config::OperationTime* cin)
    {
        mNodeId = cin->GetNodeID().second;
        mType = cin->GetType().second;
        if (mType != jvs::op_time_type_e::FROM_MODLINK)
        {
            return;
        }
        ASSERT((mType == jvs::op_time_type_e::FROM_MODLINK), "UNSUPPORTED AGGREGATE TYPE FOR OPERATION TIME");
        
        mCriterion  = cin->GetCriterion();
        mOperator   = cin->GetOperator();

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        for (auto& node : nodeStore)
        {
            if (node.first == mNodeId)
            {
                mVectorNodeReference.emplace_back(node);
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
            4 * KILLOBYTE,	     // Stack memory size to allocate
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
        uint32_t statusReportMillis = millis(); 

        mPublishTimer.LastTime = GetTimestamp();
        mPublishTimer.NextTime = CalculateTimestampNextMinuteStarts(mPublishTimer.LastTime);
        
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        time_t processingTime = 0;
 
        while (true)
        {

            if ((millis() - statusReportMillis) > (590 * SECOND_IN_MILLIS))
            {
                statusReportMillis = millis();
                size_t RemainedStackSize = uxTaskGetStackHighWaterMark(NULL);
      
                LOG_DEBUG(logger, "[OpTimeTask] Stack Remaind: %u Bytes", RemainedStackSize);
                
                deviceStatus.SetTaskRemainedStack(task_name_e::OPERATION_TIME_TASK, RemainedStackSize);
            }

            bool publishFlag = false; 

            if (GetTimestamp() > mPublishTimer.NextTime)
            {
                publishFlag = true;
                mPublishTimer.LastTime = mPublishTimer.NextTime;
                mPublishTimer.NextTime = CalculateTimestampNextMinuteStarts(mPublishTimer.LastTime);
           }
            
            for (auto& nodeReference : mVectorNodeReference)
            {
                const size_t dataStoredCount = nodeReference.get().second->VariableNode.RetrieveCount();
                if (dataStoredCount == 0)
                {
                    break;
                }
                ASSERT((dataStoredCount != 0), "THERE MUST BE DATA COLLECTED TO AGGREGATE PRODUCTION INFO");

                auto& reference = nodeReference.get();
                const im::var_data_t datum = reference.second->VariableNode.RetrieveData();
                if (datum.StatusCode != Status::Code::GOOD)
                {
                    break;
                }

                bool isStatusProcessing = false;
                bool currentErrorStatus = alarmMonitor.HasError();
                updateOperationTime(datum, *reference.second, &isStatusProcessing);
                
                if (isStatusProcessing == true)
                {
                    if (currentErrorStatus == false)
                    {
                        if (mStatus != jvs::op_status_e::PROCESSING)
                        {
                            mStatus = jvs::op_status_e::PROCESSING;
                            publishOperationStatus();
                        }
                        ++processingTime;
                    }
                }
                else
                {
                    if (mStatus != jvs::op_status_e::IDLE)
                    {
                        mStatus = jvs::op_status_e::IDLE;
                        publishOperationStatus();
                    }
                }

                if (publishFlag)
                {
                    publishInfo(processingTime);
                    processingTime = 0;
                }
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void OperationTime::updateOperationTime(const im::var_data_t& datum, im::Node& node, bool* isStatusProcessing)
    {
        switch (mOperator.second)
        {
        case jvs::cmp_op_e::EQUAL:
            *isStatusProcessing = strategyEqual(datum, node);
            break;
        case jvs::cmp_op_e::GREATER_EQUAL:
            *isStatusProcessing = strategyGreaterOrEqual(datum, node);
            break;
        case jvs::cmp_op_e::GREATER_THAN:
            *isStatusProcessing = strategyGreaterThan(datum, node);
            break;
        case jvs::cmp_op_e::LESS_EQUAL:
            *isStatusProcessing = strategyLessOrEqual(datum, node);
            break;
        case jvs::cmp_op_e::LESS_THAN:
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
        case jvs::dt_e::BOOLEAN:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Boolean);

        case jvs::dt_e::INT8:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int8);

        case jvs::dt_e::UINT8:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt8);

        case jvs::dt_e::INT16:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int16);

        case jvs::dt_e::UINT16:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt16);

        case jvs::dt_e::INT32:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int32);

        case jvs::dt_e::UINT32:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt32);

        case jvs::dt_e::INT64:
            return mCriterion.second == static_cast<int32_t>(datum.Value.Int64);

        case jvs::dt_e::UINT64:
            return mCriterion.second == static_cast<int32_t>(datum.Value.UInt64);

        case jvs::dt_e::FLOAT32:
            return mCriterion.second == datum.Value.Float32;

        case jvs::dt_e::FLOAT64:
            return mCriterion.second == datum.Value.Float64;
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyGreaterOrEqual(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Boolean);

        case jvs::dt_e::INT8:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int8);

        case jvs::dt_e::UINT8:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt8);

        case jvs::dt_e::INT16:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int16);

        case jvs::dt_e::UINT16:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt16);

        case jvs::dt_e::INT32:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int32);

        case jvs::dt_e::UINT32:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt32);

        case jvs::dt_e::INT64:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.Int64);

        case jvs::dt_e::UINT64:
            return mCriterion.second >= static_cast<int32_t>(datum.Value.UInt64);

        case jvs::dt_e::FLOAT32:
            return mCriterion.second >= datum.Value.Float32;

        case jvs::dt_e::FLOAT64:
            return mCriterion.second >= datum.Value.Float64;
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyGreaterThan(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Boolean);

        case jvs::dt_e::INT8:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int8);

        case jvs::dt_e::UINT8:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt8);

        case jvs::dt_e::INT16:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int16);

        case jvs::dt_e::UINT16:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt16);

        case jvs::dt_e::INT32:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int32);

        case jvs::dt_e::UINT32:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt32);

        case jvs::dt_e::INT64:
            return mCriterion.second > static_cast<int32_t>(datum.Value.Int64);

        case jvs::dt_e::UINT64:
            return mCriterion.second > static_cast<int32_t>(datum.Value.UInt64);

        case jvs::dt_e::FLOAT32:
            return mCriterion.second > datum.Value.Float32;

        case jvs::dt_e::FLOAT64:
            return mCriterion.second > datum.Value.Float64;
            
        default:
            return false;
        }
    }

    bool OperationTime::strategyLessOrEqual(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Boolean);

        case jvs::dt_e::INT8:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int8);

        case jvs::dt_e::UINT8:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt8);

        case jvs::dt_e::INT16:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int16);

        case jvs::dt_e::UINT16:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt16);

        case jvs::dt_e::INT32:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int32);

        case jvs::dt_e::UINT32:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt32);

        case jvs::dt_e::INT64:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.Int64);

        case jvs::dt_e::UINT64:
            return mCriterion.second <= static_cast<int32_t>(datum.Value.UInt64);

        case jvs::dt_e::FLOAT32:
            return mCriterion.second <= datum.Value.Float32;

        case jvs::dt_e::FLOAT64:
            return mCriterion.second <= datum.Value.Float64;

        default:
            return false;
        }
    }

    bool OperationTime::strategyLessThan(const im::var_data_t& datum, im::Node& node)
    {
        switch (datum.DataType)
        {
        case jvs::dt_e::BOOLEAN:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Boolean);

        case jvs::dt_e::INT8:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int8);

        case jvs::dt_e::UINT8:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt8);

        case jvs::dt_e::INT16:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int16);

        case jvs::dt_e::UINT16:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt16);

        case jvs::dt_e::INT32:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int32);

        case jvs::dt_e::UINT32:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt32);

        case jvs::dt_e::INT64:
            return mCriterion.second < static_cast<int32_t>(datum.Value.Int64);

        case jvs::dt_e::UINT64:
            return mCriterion.second < static_cast<int32_t>(datum.Value.UInt64);

        case jvs::dt_e::FLOAT32:
            return mCriterion.second < datum.Value.Float32;

        case jvs::dt_e::FLOAT64:
            return mCriterion.second < datum.Value.Float64;
            
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
        const size_t size = UINT8_MAX;
        char payload[size] = {'\0'};
        json.Serialize(production, size, payload);

        mqtt::Message message(mqtt::topic_e::UPTIME, payload);
        mqtt::cdo.Store(message);
    }

    void OperationTime::publishOperationStatus()
    {
        operation_struct_t status;

        status.SourceTimestamp = GetTimestampInMillis();
        status.Status = mStatus == jvs::op_status_e::PROCESSING ? "processing" : "idle";
        status.Topic = mqtt::topic_e::OPERATION;

        JSON json;
        const size_t size = 128;
        char payload[size] = {'\0'};
        json.Serialize(status, size, payload);

        mqtt::Message message(mqtt::topic_e::OPERATION, payload);
        mqtt::cdo.Store(message);
    }



    OperationTime* OperationTime::mInstance = nullptr;
}