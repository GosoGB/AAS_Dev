/**
 * @file DeprecableAlarm.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 정보 모니터링 및 생성에 사용되는 클래스를 선언합니다.
 * @note MUFFIN Ver.0.0.1 개발 이후 또는 정보 모델 구현이 완료되면 재설계 할 예정입니다.
 * 
 * @date 2024-10-28
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
#include "Jarvis/Config/Information/Alarm.h"



namespace muffin {

    class AlarmMonitor
    {
    public:
        AlarmMonitor(AlarmMonitor const&) = delete;
        void operator=(AlarmMonitor const&) = delete;
        static AlarmMonitor& GetInstance();
    private:
        AlarmMonitor();
        virtual ~AlarmMonitor();
    private:
        static AlarmMonitor* mInstance;
    
    public:
        void Add(jarvis::config::Alarm* cin);
        void Clear();
    public:
        void StartTask();
        void StopTask();
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        void strategyLCL(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyUCL(const jarvis::config::Alarm cin, const im::var_data_t datum, const im::Variable& node);
        void strategyLclAndUcl(const jarvis::config::Alarm cin, const im::var_data_t datum, const im::Variable& node);
        void strategyCondition(const jarvis::config::Alarm cin, const im::var_data_t datum, const im::Variable& node);
        bool isActiveAlarm(const std::string& uid);
        float convertToFloat(const im::var_data_t& datum);
        alarm_struct_t retrieveActiveAlarm(const std::string& uid);
        std::string createAlarmUUID();
        void activateAlarm(const bool isLCL, const jarvis::config::Alarm cin, const im::Variable& node);
        void deactivateAlarm(const bool isLCL, const jarvis::config::Alarm cin);
    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node>>;
        std::vector<jarvis::config::Alarm> mVectorConfig;
        std::vector<node_reference> mVectorNodeReference;
        std::vector<alarm_struct_t> mVectorAlarmInfo;
        TaskHandle_t xHandle;
    };
}