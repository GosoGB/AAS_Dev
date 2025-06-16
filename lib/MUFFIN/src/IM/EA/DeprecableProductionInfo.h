/**
 * @file DeprecableProductionInfo.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 생산실적 정보를 집계하고 매분 서버로 전송하는 클래스를 선언합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
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
#include "JARVIS/Config/Information/Production.h"



namespace muffin {

    typedef struct ProductionCountType
    {
        uint64_t Total  = 0;
        uint64_t Good   = 0;
        uint64_t NG     = 0;
    } product_count_t;

    class ProductionInfo
    {
    public:
        ProductionInfo(ProductionInfo const&) = delete;
        void operator=(ProductionInfo const&) = delete;
        static ProductionInfo& GetInstance();
    private:
        ProductionInfo();
        virtual ~ProductionInfo();
    private:
        static ProductionInfo* mInstance;
    
    public:
        void Config(jvs::config::Production* cin);
        void Clear();
    private:
        std::pair<bool, std::string> mNodeIdTotal;
        std::pair<bool, std::string> mNodeIdGood;
        std::pair<bool, std::string> mNodeIdNG;

    public:
        void StartTask();
        void StopTask();
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        void updateCountTotal(const im::var_data_t& datum);
        void publishInfo(const uint64_t productionValue); 
        // void updateCountGood(const im::var_data_t& datum);
        // void updateCountNG(const im::var_data_t& datum);
    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node*>>;
        std::vector<node_reference> mVectorNodeReference;
        product_count_t mProductCount;
        product_count_t mPreviousCount;
    private:
        TaskHandle_t xHandle;
    };
}