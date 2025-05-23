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
        bool isContainNode(std::string nid);
    public:
        void StartTask();
        void StopTask();

    public:
        Status ConvertUCL(std::string nid, std::string ucl);
        Status ConvertLCL(std::string nid, std::string lcl);
    private:
        static void wrapImplTask(void* pvParams);
        void implTask();
        void strategyLCL(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyUCL(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyLclAndUcl(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        void strategyCondition(const jvs::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node);
        bool isActiveAlarm(const std::string& nid, const jvs::alarm_pub_type_e type);
        float convertToFloat(const im::var_data_t& datum);
        json_alarm_t retrieveActiveAlarm(const std::string& nid);
        std::string createAlarmUUID();
        void activateAlarm(const jvs::alarm_type_e type, const jvs::config::Alarm cin, const im::Variable& node, const std::string& value);
        void deactivateAlarm(const jvs::alarm_type_e type, const jvs::config::Alarm cin, const std::string& value);

    private:
        Status updateFlashUclValue(std::string nodeid, float ucl);
        Status updateFlashLclValue(std::string nodeid, float lcl);
    private:
        using node_reference = std::reference_wrapper<std::pair<const std::string, im::Node*>>;
        std::vector<jvs::config::Alarm> mVectorConfig;
        std::vector<node_reference> mVectorNodeReference;
        std::vector<json_alarm_t> mVectorAlarmInfo;
        TaskHandle_t xHandle;
    };
}