/**
 * @file DeprecableProductionInfo.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */



#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <string>
#include <vector>

#include "DataFormat/JSON/JSON.h"
#include "IM/Node/NodeStore.h"
#include "Jarvis/Config/Information/Production.h"



namespace muffin {

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
        void Config(jarvis::config::Production* cin);
    public:
        void StartTask();
        void StopTask();

    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node>>;
        std::vector<node_reference> mVectorNodeReference;
      
        std::string mTotalNodeID;
        std::string mGoodNodeID;
        std::string mNotGoodNodeID;

        uint64_t mTotalProductionValue;
        uint64_t mPreviousTotalValue = UINT64_MAX;
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        
    private:
        TaskHandle_t xHandle;
        
    };
}