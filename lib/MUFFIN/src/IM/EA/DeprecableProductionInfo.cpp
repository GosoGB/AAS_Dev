/**
 * @file DeprecableProductionInfo.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 생산실적 정보를 집계하고 매분 서버로 전송하는 클래스를 정의합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
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
        : mNodeIdTotal(std::make_pair(false, ""))
        , mNodeIdGood(std::make_pair(false, ""))
        , mNodeIdNG(std::make_pair(false, ""))
        , xHandle(NULL)
    {
    }
    
    ProductionInfo::~ProductionInfo()
    {
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
            wrapImplTask,      // Function to be run inside of the task
            "ProdInfoTask",    // The identifier of this task for men
            4 * 1024,	       // Stack memory size to allocate
            this,	           // Task parameters to be passed to the function
            0,		           // Task Priority for scheduling
            &xHandle,          // The identifier of this task for machines
            1	               // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            // LOG_INFO(logger, "The ProductionInfo task has been started");
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
            LOG_WARNING(logger, "NO PRODUCTION INFO TASK TO STOP!");
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
        const uint16_t remainedStackCheckInterval = 6 * 1000;
    #endif

        time_t LastTime;
        time_t NextTime = CalculateTimestampNextMinuteStarts(GetTimestamp());
  
        while (true)
        {
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                //LOG_DEBUG(logger, "[TASK: ProductionInfo] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(xHandle));
                checkRemainedStackMillis = millis();
            }
        #endif

            for (auto& nodeReference : mVectorNodeReference)
            {
                const size_t dataStoredCount = nodeReference.get().second->VariableNode.RetrieveCount();
                if (dataStoredCount == 0)
                {
                    break;
                }
                ASSERT((dataStoredCount != 0), "THERE MUST BE DATA COLLECTED TO AGGREGATE PRODUCTION INFO");

/**
 * @todo VariableNode 내부에 mDataBuffer가 비어있을 시 메모리 크래시가 발생함. 해결 방법을 찾아봐야함.
 * @note mDataBuffer가 비어있지 않을 때만 들어오도록 수정하였음. 크래시 없을 시 위의 todo는 제거할 에정임
 */
                auto& reference = nodeReference.get();
                const im::var_data_t datum = reference.second->VariableNode.RetrieveData();
                if (datum.StatusCode != Status::Code::GOOD)
                {
                    break;
                }

                /**
                 * @todo PROGIX에 총 생산수량 외에 양품수량, 불량수량을 보내기 위한
                 *       메시지 형식을 클라우드 개발팀과 협의해야 합니다.
                 */
                if (mNodeIdTotal.second == reference.first)
                {
                    updateCountTotal(datum);
                }
                else if (mNodeIdGood.second == reference.first)
                {
                    // updateCountGood(datum);
                }
                else if (mNodeIdNG.second == reference.first)
                {
                    // updateCountNG(datum);
                }

                if (GetTimestamp() < NextTime)
                {
                    break;
                }

                LastTime = NextTime;
                NextTime = CalculateTimestampNextMinuteStarts(LastTime);

                publishInfo(mProductCount.Total);
                mProductCount.Total = 0;
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void ProductionInfo::updateCountTotal(const im::var_data_t& datum)
    {
        uint64_t TotalValue = 0;

        switch (datum.DataType)
        {
        case jvs::dt_e::UINT8:
            TotalValue = datum.Value.UInt8;
            break;
        case jvs::dt_e::UINT16:
            TotalValue = datum.Value.UInt16;
            break;
        case jvs::dt_e::UINT32:
            TotalValue = datum.Value.UInt32;
            break;
        case jvs::dt_e::UINT64:
            TotalValue = datum.Value.UInt64;
            break;
        default:
            /**
             * @todo JARVIS Validator 쪽에서 설정 형식 간 선결조건, 관계 검증에 추가해야 합니다.
             */
            ASSERT(false, "PRODUCT COUNT MUST BE A UNSIGNED INTEGER VALUE: %s", mNodeIdTotal.second.c_str());
            break;
        }


        if (mPreviousCount.Total == TotalValue)
        {
            return;
        }
        
        if (mPreviousCount.Total > TotalValue)
        {
             mProductCount.Total = 0;
        }
        else
        {
            mProductCount.Total =  mProductCount.Total + TotalValue - mPreviousCount.Total;
        }
       
        mPreviousCount.Total = TotalValue;

    }

    void ProductionInfo::Config(jvs::config::Production* cin)
    {
        std::pair<Status,std::string> ret = cin->GetNodeIdTotal();
        if (ret.first == Status::Code::GOOD)
        {
            mNodeIdTotal.first = true;
            mNodeIdTotal.second = ret.second;
        }

        ret = cin->GetNodeIdGood();
        if (ret.first == Status::Code::GOOD)
        {
            mNodeIdTotal.first = true;
            mNodeIdGood.second = ret.second;
        }
        
        ret = cin->GetNodeIdNG();
        if (ret.first == Status::Code::GOOD)
        {
            mNodeIdTotal.first = true;
            mNodeIdNG.second = ret.second;
        }

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        for (auto& node : nodeStore)
        {
            if ((node.first == mNodeIdTotal.second) || 
                (node.first == mNodeIdGood.second)  || 
                (node.first == mNodeIdNG.second))
            {
                mVectorNodeReference.emplace_back(node);
            }
        }
    }
    
    void ProductionInfo::Clear()
    {
        mNodeIdTotal.first = false;
        mNodeIdGood.first = false;
        mNodeIdNG.first = false;
        
        mProductCount.Total = 0;
        mProductCount.Good = 0;
        mProductCount.NG = 0;

        mPreviousCount.Total = 0;
        mPreviousCount.Good = 0;
        mPreviousCount.NG = 0;

        mVectorNodeReference.clear();

        // LOG_INFO(logger, "Production Info configurations and data have been cleared");
    }

    void ProductionInfo::publishInfo(const uint64_t productionValue)
    {
        if (productionValue == 0)
        {
            return;
        }
        
        progix_struct_t production;

        production.SourceTimestamp = TimestampToExactHourKST();
        production.Value = std::to_string(productionValue);
        production.Topic = mqtt::topic_e::FINISHEDGOODS;

        JSON json;
        const std::string payload = json.Serialize(production);
        mqtt::Message message(mqtt::topic_e::FINISHEDGOODS, payload);

        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        cdo.Store(message);
    }


    ProductionInfo* ProductionInfo::mInstance = nullptr;
}