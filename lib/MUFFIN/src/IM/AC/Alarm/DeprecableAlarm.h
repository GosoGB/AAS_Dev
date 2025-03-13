/**
 * @file DeprecableAlarm.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 정보 모니터링 및 생성에 사용되는 클래스를 선언합니다.
 * @note MUFFIN Ver.0.0.1 개발 이후 또는 정보 모델 구현이 완료되면 재설계 할 예정입니다.
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
#include "JARVIS/Config/Information/Alarm.h"



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
        void Add(jvs::config::Alarm* cin);
        void Clear();
        bool HasError() const;
    public:
        void StartTask();
        void StopTask();

    public:
        std::pair<bool,std::vector<std::string>> GetUclUid();
        std::pair<bool,std::vector<std::string>> GetLclUid();
        bool ConvertUCL(std::string ucluid, std::string ucl);
        bool ConvertLCL(std::string lcluid, std::string lcl);
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        void strategyLCL(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyUCL(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyLclAndUcl(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyCondition(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        bool isActiveAlarm(const std::string& uid);
        float convertToFloat(const im::var_data_t& datum);
        alarm_struct_t retrieveActiveAlarm(const std::string& uid);
        std::string createAlarmUUID();
        void activateAlarm(const jvs::alarm_type_e type, const jvs::config::Alarm cin, const im::Variable& node, const std::string& value);
        void deactivateAlarm(const jvs::alarm_type_e type, const jvs::config::Alarm cin, const std::string& value);

    private:
        void pubLclToScautr( jvs::config::Alarm& cin, const im::Variable& node);
        void pubUclToScautr( jvs::config::Alarm& cin, const im::Variable& node);
        void updateFlashUclValue(std::string nodeid, float ucl);
        void updateFlashLclValue(std::string nodeid, float lcl);
    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node*>>;
        std::vector<jvs::config::Alarm> mVectorConfig;
        std::vector<node_reference> mVectorNodeReference;
        std::vector<alarm_struct_t> mVectorAlarmInfo;
        TaskHandle_t xHandle;
    };
}