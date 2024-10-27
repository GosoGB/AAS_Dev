/**
 * @file DeprecableAlarm.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 정보 모니터링 및 생성에 사용되는 클래스를 정의합니다.
 * @note MUFFIN Ver.0.0.1 개발 이후 또는 정보 모델 구현이 완료되면 재설계 할 예정입니다.
 * 
 * @date 2024-10-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DeprecableAlarm.h"
#include "Protocol/MQTT/CDO.h"



namespace muffin {

    AlarmMonitor& AlarmMonitor::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) AlarmMonitor();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR MQTT CDO");
            }
        }

        return *mInstance;
    }
    
    AlarmMonitor::AlarmMonitor()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    AlarmMonitor::~AlarmMonitor()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    void AlarmMonitor::Add(jarvis::config::Alarm* cin)
    {
        mVectorConfig.emplace_back(*cin);

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();

        for (auto& node : nodeStore)
        {
            if (node.first == cin->GetNodeID().second)
            {
                mVectorNodeReference.emplace_back(node);
                return;
            }
        }
    }
    
    void AlarmMonitor::Clear()
    {
        mVectorConfig.clear();
        mVectorNodeReference.clear();
    }

    void AlarmMonitor::StartTask()
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
            4 * 1024,	   // Stack memory size to allocate
            this,	       // Task parameters to be passed to the function
            0,		       // Task Priority for scheduling
            &xHandle,      // The identifier of this task for machines
            0	           // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The Alarm task has been started");
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

    void AlarmMonitor::StopTask()
    {
        if (xHandle == NULL)
        {
            LOG_WARNING(logger, "NO ALARM TASK TO STOP!");
            return;
        }
        
        vTaskDelete(xHandle);
        xHandle = NULL;
    }

    void AlarmMonitor::wrapImplTask(void* pvParams)
    {
        static_cast<AlarmMonitor*>(pvParams)->implTask();
    }

    void AlarmMonitor::implTask()
    {
        while (true)
        {
            for (auto& cin : mVectorConfig)
            {
                const std::string nodeId = cin.GetNodeID().second;

                for (auto& nodeReference : mVectorNodeReference)
                {
                    auto& reference = nodeReference.get();
                    if (nodeId != reference.first)
                    {
                        continue;
                    }

                    im::var_data_t datum = reference.second.VariableNode.RetrieveData();
                    if (datum.StatusCode != Status::Code::GOOD)
                    {
                        break;
                    }

                    switch (cin.GetType().second)
                    {
                    case jarvis::alarm_type_e::ONLY_LCL:
                        strategyLCL(cin, datum, reference.second.VariableNode);
                        break;
                    case jarvis::alarm_type_e::ONLY_UCL:
                        strategyUCL(cin, datum);
                        break;
                    case jarvis::alarm_type_e::LCL_AND_UCL:
                        strategyLclAndUcl(cin, datum);
                        break;
                    case jarvis::alarm_type_e::CONDITION:
                        strategyCondition(cin, datum);
                        break;
                    default:
                        break;
                    }
                }
            }
        
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }

    void AlarmMonitor::strategyLCL(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node)
    {
        ASSERT((datum.DataType != jarvis::dt_e::BOOLEAN), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF BOOLEAN DATA TYPE");
        ASSERT((datum.DataType != jarvis::dt_e::STRING), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF STRING DATA TYPE");

        const float lcl = cin.GetLCL().second;
        float value = 0.0f;
        bool isAlarm = false;
        
        /**
         * @todo double 타입을 처리할 수 있도록 코드를 수정해야 합니다.
         */
        switch (datum.DataType)
        {
        case jarvis::dt_e::INT8:
            value = static_cast<float>(datum.Value.Int8);
            break;
        case jarvis::dt_e::UINT8:
            value = static_cast<float>(datum.Value.UInt8);
            break;
        case jarvis::dt_e::INT16:
            value = static_cast<float>(datum.Value.Int16);
            break;
        case jarvis::dt_e::UINT16:
            value = static_cast<float>(datum.Value.UInt16);
            break;
        case jarvis::dt_e::INT32:
            value = static_cast<float>(datum.Value.Int32);
            break;
        case jarvis::dt_e::UINT32:
            value = static_cast<float>(datum.Value.UInt32);
            break;
        case jarvis::dt_e::INT64:
            value = static_cast<float>(datum.Value.Int64);
            break;
        case jarvis::dt_e::UINT64:
            value = static_cast<float>(datum.Value.UInt64);
            break;
        case jarvis::dt_e::FLOAT32:
            value = static_cast<float>(datum.Value.Float32);
            break;
        case jarvis::dt_e::FLOAT64:
            value = static_cast<float>(datum.Value.Float64);
            break;
        default:
            return;
        }

        const bool isNewAlarm = isActiveAlarm(cin.GetLclAlarmUID().second) == false;
        const bool isAlarmCondition = value < lcl;

        if (isNewAlarm == true)
        {
            if (isAlarmCondition == true)
            {
                alarm_struct_t alarm;
                alarm.Topic = mqtt::topic_e::ALARM;
                alarm.AlarmType = "start";
                alarm.AlarmStartTime = GetTimestampInMillis();
                alarm.AlarmFinishTime = -1;
                alarm.Name = node.GetDisplayName();
                alarm.Uid = cin.GetLclAlarmUID().second;
                alarm.UUID = createAlarmUUID();

                mVectorAlarmInfo.emplace_back(alarm);

                JSON json;
                const std::string payload = json.Serialize(alarm);
                mqtt::Message message(mqtt::topic_e::ALARM, payload);

                mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                cdo.Store(message);

                LOG_INFO(logger, "[Alarm][Payload] %s", payload.c_str());
            }
            else
            {
                return;
            }
        }
        else
        {
            if (isAlarmCondition == true)
            {
                return;
            }
            else
            {
                alarm_struct_t alarm = retrieveActiveAlarm(cin.GetLclAlarmUID().second);
                alarm.AlarmFinishTime = GetTimestampInMillis();
                alarm.AlarmType = "finish";

                JSON json;
                const std::string payload = json.Serialize(alarm);
                mqtt::Message message(mqtt::topic_e::ALARM, payload);

                mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                cdo.Store(message);

                LOG_INFO(logger, "[Alarm][Payload] %s", payload.c_str());
            }
        }
    }

    void AlarmMonitor::strategyUCL(jarvis::config::Alarm cin, const im::var_data_t datum)
    {
        ;
    }

    void AlarmMonitor::strategyLclAndUcl(jarvis::config::Alarm cin, const im::var_data_t datum)
    {
        ;
    }

    void AlarmMonitor::strategyCondition(jarvis::config::Alarm cin, const im::var_data_t datum)
    {
        ;
    }

    bool AlarmMonitor::isActiveAlarm(const std::string& uid)
    {
        for (auto& alarmInfo : mVectorAlarmInfo)
        {
            if (uid == alarmInfo.Uid)
            {
                return true;
            }
        }

        return false;
    }

    alarm_struct_t AlarmMonitor::retrieveActiveAlarm(const std::string& uid)
    {
        // auto it = std::find(mVectorAlarmInfo.begin(), mVectorAlarmInfo.end(), [uid](const alarm_struct_t& alarm)
        // {
        //     return alarm.Uid == uid;
        // });

        alarm_struct_t alarm;

        for (auto it = mVectorAlarmInfo.begin(); it != mVectorAlarmInfo.end(); ++it)
        {
            alarm = *it;
            mVectorAlarmInfo.erase(it);
        }
        
        // alarm_struct_t alarm = *it;
        // mVectorAlarmInfo.erase(it);
        return alarm;
    }

    void IntToHex(const uint32_t inInt, char* returnVar)
    {
        const char* HEXMAP = "0123456789abcdef";
        for (int i = 0; i < 8; i++)
        {
            // Shift each hex digit to the right, and then map it to its corresponding value
            returnVar[7 - i] = HEXMAP[(inInt >> (i * 4)) & 0b1111];
        }
    }

    void CreateUUID(char* returnUUID)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint32_t chunk = esp_random();
            
            if (i == 1)
            {
                chunk &= 0xFFFF0FFF;
                chunk |= 0x00004000;
            }
            else if (i == 2)
            {
                chunk &= 0b00111111111111111111111111111111;
                chunk |= 0b10000000000000000000000000000000;
            }

            char chunkChars[8] = { 0 };
            IntToHex(chunk, chunkChars);
            for (int p = 0; p < 8; p++)
            {
                returnUUID[p + 8 * i] = chunkChars[p];
            }
        }

        int dashOffset = 4;
        const int UUID_NUM_DIGITS = 32;
        for (int i = UUID_NUM_DIGITS - 1; i >= 0; --i)
        {
            if (i == 7 || i == 11 || i == 15 || i == 19)
            {
                returnUUID[i + dashOffset--] = '-';
            }
            returnUUID[i + dashOffset] = returnUUID[i];
        }
        returnUUID[36] = 0;
    }

    std::string AlarmMonitor::createAlarmUUID()
    {
        char returnUUID[37];
        CreateUUID(returnUUID);
        return std::string(returnUUID).substr(0, 12);
    }


    AlarmMonitor* AlarmMonitor::mInstance = nullptr;
}