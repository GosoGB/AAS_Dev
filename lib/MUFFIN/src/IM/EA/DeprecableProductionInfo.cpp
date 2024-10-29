/**
 * @file DeprecableProductionInfo.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DeprecableProductionInfo.h"
#include "Protocol/MQTT/CDO.h"




namespace muffin {

    ProductionInfo& ProductionInfo::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) ProductionInfo();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR PRODUCTION INFO");
            }
        }

        return *mInstance;
    }
    
    ProductionInfo::ProductionInfo()
        : xHandle(NULL)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ProductionInfo::~ProductionInfo()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void ProductionInfo::StartTask()
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
            wrapImplTask,  // Function to be run inside of the task
            "implTask",    // The identifier of this task for men
            8 * 1024,	   // Stack memory size to allocate
            this,	       // Task parameters to be passed to the function
            0,		       // Task Priority for scheduling
            &xHandle,      // The identifier of this task for machines
            1	           // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The ProductionInfo task has been started");
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

    void ProductionInfo::StopTask()
    {
        if (xHandle == NULL)
        {
            LOG_WARNING(logger, "NO ALARM TASK TO STOP!");
            return;
        }
        
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    void ProductionInfo::wrapImplTask(void* pvParams)
    {
        static_cast<ProductionInfo*>(pvParams)->implTask();
    }

    void ProductionInfo::implTask()
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        while (true)
        {
      
#ifdef DEBUG
    if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
    {
        LOG_DEBUG(logger, "[TASK: ProductionInfo] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(xHandle));
        checkRemainedStackMillis = millis();
    }
#endif
        LOG_DEBUG(logger,"mVectorNodeReference size : %d", mVectorNodeReference.size());

        for (auto& nodeReference : mVectorNodeReference)
        {
     
            auto& reference = nodeReference.get();
    
            if (mTotalNodeID == reference.first)
            {
                // VariableNode 내부에 mDataBuffer가 업데이트 안되어있을 시 메모리 크래시가 발생함, 해결방법 찾아봐야함
                im::var_data_t datum = reference.second.VariableNode.RetrieveData();
  
                if (datum.StatusCode != Status::Code::GOOD)
                {
                    break;
                }
              
                switch (datum.DataType)
                {
                case jarvis::dt_e::UINT8:
                    mTotalProductionValue = datum.Value.UInt8;
                    break;
                case jarvis::dt_e::UINT16:
                    mTotalProductionValue = datum.Value.UInt16;
                    break;
                case jarvis::dt_e::UINT32:
                    mTotalProductionValue = datum.Value.UInt32;
                    break;
                case jarvis::dt_e::UINT64:
                    mTotalProductionValue = datum.Value.UInt64;
                    break;
                default:
                    break;
                }

                if (mPreviousTotalValue > mTotalProductionValue)
                {
                    mTotalProductionValue = 0;
                }
                else
                {
                    mTotalProductionValue = mTotalProductionValue - mPreviousTotalValue;
                }
                
                mPreviousTotalValue = mTotalProductionValue;

                progix_struct_t production;

                production.SourceTimestamp = TimestampToExactHourKST();
                production.Value = mTotalProductionValue;
                production.Topic = mqtt::topic_e::FINISHEDGOODS;

                JSON json;
                const std::string payload = json.Serialize(production);
                mqtt::Message message(mqtt::topic_e::FINISHEDGOODS, payload);

                mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                cdo.Store(message);

                LOG_INFO(logger, "[PRODUCTIONINFO] %s", payload.c_str());
            
            }
        }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }


    void ProductionInfo::Config(jarvis::config::Production* cin)
    {
        std::pair<Status,std::string> ret = cin->GetNodeIdTotal();
        
        if (ret.first == Status::Code::GOOD)
        {
            mTotalNodeID = ret.second;

            LOG_DEBUG(logger,"mTotalNodeID : %s", mTotalNodeID.c_str());
        }

        ret = cin->GetNodeIdGood();

        if (ret.first == Status::Code::GOOD)
        {
            mGoodNodeID = ret.second;
            LOG_DEBUG(logger,"mGoodNodeID : %s", mTotalNodeID.c_str());
        }
        
        ret = cin->GetNodeIdNG();

        if (ret.first == Status::Code::GOOD)
        {
            mNotGoodNodeID = ret.second;
            LOG_DEBUG(logger,"mNotGoodNodeID : %s", mTotalNodeID.c_str());
        }

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();

        for (auto& node : nodeStore)
        {
            if ((node.first == mTotalNodeID)  ||
                (node.first == mGoodNodeID)   ||
                (node.first == mNotGoodNodeID) )
            {
                mVectorNodeReference.emplace_back(node);
    
            }
            
        }
        LOG_DEBUG(logger,"mVectorNodeReference size : %d", mVectorNodeReference.size());
    }

    ProductionInfo* ProductionInfo::mInstance = nullptr;
}