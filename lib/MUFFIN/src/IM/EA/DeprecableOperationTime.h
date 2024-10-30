/**
 * @file DeprecableOperationTime.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 가동시간 정보를 집계하고 매분 서버로 전송하는 클래스를 선언합니다.
 * 
 * @date 2024-10-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <string>
#include <vector>

#include "DataFormat/JSON/JSON.h"
#include "IM/Node/NodeStore.h"
#include "Jarvis/Config/Information/OperationTime.h"



namespace muffin {

    typedef struct ProductionInfoPublishTimer
    {
        time_t LastTime;
        time_t NextTime;
    } pub_timer;

    class OperationTime
    {
    public:
        OperationTime(OperationTime const&) = delete;
        void operator=(OperationTime const&) = delete;
        static OperationTime& GetInstance();
    private:
        OperationTime();
        virtual ~OperationTime();
    private:
        static OperationTime* mInstance;
    
    public:
        void Config(jarvis::config::OperationTime* cin);
        void Clear();
    private:
        std::string mNodeId;
        jarvis::op_time_type_e mType;
        std::pair<Status, int32_t> mCriterion;
        std::pair<Status, jarvis::cmp_op_e> mOperator;

    public:
        void StartTask();
        void StopTask();
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        void updateOperationTime(const im::var_data_t& datum, im::Node& node, bool* isStatusProcessing);
        bool strategyEqual(const im::var_data_t& datum, im::Node& node);
        bool strategyGreaterOrEqual(const im::var_data_t& datum, im::Node& node);
        bool strategyGreaterThan(const im::var_data_t& datum, im::Node& node);
        bool strategyLessOrEqual(const im::var_data_t& datum, im::Node& node);
        bool strategyLessThan(const im::var_data_t& datum, im::Node& node);
        void publishInfo(const time_t processingTime);
    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node>>;
        std::vector<node_reference> mVectorNodeReference;
    private:
        TaskHandle_t xHandle;
        pub_timer mPublishTimer;
    };
}