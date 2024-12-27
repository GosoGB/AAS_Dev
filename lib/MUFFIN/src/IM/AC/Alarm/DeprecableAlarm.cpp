/**
 * @file DeprecableAlarm.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 알람 정보 모니터링 및 생성에 사용되는 클래스를 정의합니다.
 * @note MUFFIN Ver.0.0.1 개발 이후 또는 정보 모델 구현이 완료되면 재설계 할 예정입니다.
 * 
 * @date 2024-10-28
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include "Storage/ESP32FS/ESP32FS.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DeprecableAlarm.h"
#include "Protocol/MQTT/CDO.h"

#include "Common/Convert/ConvertClass.h"


namespace muffin {

    AlarmMonitor& AlarmMonitor::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) AlarmMonitor();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR ALARM MONITOR");
            }
        }

        return *mInstance;
    }
    
    AlarmMonitor::AlarmMonitor()
        : xHandle(NULL)
    {
    }
    
    AlarmMonitor::~AlarmMonitor()
    {
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
        mVectorAlarmInfo.clear();
        mVectorNodeReference.clear();

        LOG_INFO(logger, "Alarm monitoring configurations and data have been cleared");
    }

    bool AlarmMonitor::HasError() const
    {
        for (auto& alarmInfo : mVectorAlarmInfo)
        {
            if (alarmInfo.Uid.at(0) == 'E')
            {
                return true;
            }
        }
        
        return false;
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
        LOG_INFO(logger, "Alarm monitoring task has been stopped");
    }

    void AlarmMonitor::wrapImplTask(void* pvParams)
    {
        static_cast<AlarmMonitor*>(pvParams)->implTask();
    }

    void AlarmMonitor::implTask()
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 6 * 1000;
    #endif

        while (true)
        {
#ifdef DEBUG
    if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
    {
        checkRemainedStackMillis = millis();
    }
#endif

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

                    if (reference.second.VariableNode.RetrieveCount() == 0)
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
                        pubLclToScautr(cin, reference.second.VariableNode);
                        strategyLCL(cin, datum, reference.second.VariableNode);
                        break;
                    case jarvis::alarm_type_e::ONLY_UCL:
                        pubUclToScautr(cin, reference.second.VariableNode);
                        strategyUCL(cin, datum, reference.second.VariableNode);
                        break;
                    case jarvis::alarm_type_e::LCL_AND_UCL:
                        pubLclToScautr(cin, reference.second.VariableNode);
                        pubUclToScautr(cin, reference.second.VariableNode);
                        strategyLclAndUcl(cin, datum, reference.second.VariableNode);
                        break;
                    case jarvis::alarm_type_e::CONDITION:
                        strategyCondition(cin, datum, reference.second.VariableNode);
                        break;
                    default:
                        break;
                    }
                }
            }


            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void AlarmMonitor::strategyLCL(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node)
    {
        ASSERT((datum.DataType != jarvis::dt_e::BOOLEAN), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF BOOLEAN DATA TYPE");
        ASSERT((datum.DataType != jarvis::dt_e::STRING), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF STRING DATA TYPE");

        const float lcl = cin.GetLCL().second;
        const float value = convertToFloat(datum);
        const jarvis::alarm_type_e type = jarvis::alarm_type_e::ONLY_LCL;
        ASSERT((cin.GetType().second == jarvis::alarm_type_e::ONLY_LCL || cin.GetType().second == jarvis::alarm_type_e::LCL_AND_UCL), "ALARM TYPE MUST INCLUDE LOWER CONTROL LIMIT");

        const bool isNewAlarm = isActiveAlarm(cin.GetLclAlarmUID().second) == false;
        const bool isAlarmCondition = value < lcl;

        if (isNewAlarm == true)
        {
            if (isAlarmCondition == true)
            {
                activateAlarm(type, cin, node);
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
                deactivateAlarm(type, cin);
            }
        }
    }

    void AlarmMonitor::strategyUCL(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node)
    {
        ASSERT((datum.DataType != jarvis::dt_e::BOOLEAN), "UCL CANNOT BE APPLIED TO VARIABLE NODE OF BOOLEAN DATA TYPE");
        ASSERT((datum.DataType != jarvis::dt_e::STRING), "UCL CANNOT BE APPLIED TO VARIABLE NODE OF STRING DATA TYPE");

        const float ucl = cin.GetUCL().second;
        const float value = convertToFloat(datum);
        const jarvis::alarm_type_e type = jarvis::alarm_type_e::ONLY_UCL;
        ASSERT((cin.GetType().second == jarvis::alarm_type_e::ONLY_UCL || cin.GetType().second == jarvis::alarm_type_e::LCL_AND_UCL), "ALARM TYPE MUST INCLUDE UPPER CONTROL LIMIT");

        const bool isNewAlarm = isActiveAlarm(cin.GetLclAlarmUID().second) == false;
        const bool isAlarmCondition = value > ucl;

        if (isNewAlarm == true)
        {
            if (isAlarmCondition == true)
            {
                activateAlarm(type, cin, node);
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
                deactivateAlarm(type, cin);
            }
        }
    }

    void AlarmMonitor::strategyLclAndUcl(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node)
    {
        ASSERT((datum.DataType != jarvis::dt_e::BOOLEAN), "LCL & UCL CANNOT BE APPLIED TO VARIABLE NODE OF BOOLEAN DATA TYPE");
        ASSERT((datum.DataType != jarvis::dt_e::STRING), "LCL & UCL CANNOT BE APPLIED TO VARIABLE NODE OF STRING DATA TYPE");
        ASSERT((cin.GetType().second == jarvis::alarm_type_e::LCL_AND_UCL), "ALARM TYPE MUST INCLUDE BOTH LOWER AND UPPER CONTROL LIMITS");

        const float ucl = cin.GetUCL().second;
        const float lcl = cin.GetLCL().second;
        const float value = convertToFloat(datum);

        const bool isNewLclAlarm = isActiveAlarm(cin.GetLclAlarmUID().second) == false;
        const bool isNewUclAlarm = isActiveAlarm(cin.GetUclAlarmUID().second) == false;
        const bool isUclCondition = value > ucl;
        const bool isLclCondition = value < lcl;

        if (isNewLclAlarm == true)
        {
            if (isLclCondition == true)
            {
                activateAlarm(jarvis::alarm_type_e::ONLY_LCL, cin, node);
            }
        }
        else
        {
            if (isLclCondition == false)
            {
                deactivateAlarm(jarvis::alarm_type_e::ONLY_LCL, cin);
            }
        }

        if (isNewUclAlarm == true)
        {
            if (isUclCondition == true)
            {
                activateAlarm(jarvis::alarm_type_e::ONLY_UCL, cin, node);
            }
        }
        else
        {
            if (isUclCondition == false)
            {
                deactivateAlarm(jarvis::alarm_type_e::ONLY_UCL, cin);
            }

        }
    }

    void AlarmMonitor::strategyCondition(const jarvis::config::Alarm& cin, const im::var_data_t& datum, const im::Variable& node)
    {
        ASSERT((cin.GetType().second == jarvis::alarm_type_e::CONDITION), "ALARM TYPE MUST BE CONDITION TYPE");

        int16_t value = 0;
        bool hasValue = false;
        switch (datum.DataType)
        {
        case jarvis::dt_e::BOOLEAN:
            value = static_cast<int16_t>(datum.Value.Boolean);
            hasValue = true;
            break;
        case jarvis::dt_e::INT8:
            value = static_cast<int16_t>(datum.Value.Int8);
            hasValue = true;
            break;
        case jarvis::dt_e::INT16:
            value = static_cast<int16_t>(datum.Value.Int16);
            hasValue = true;
            break;
        case jarvis::dt_e::UINT8:
            value = static_cast<int16_t>(datum.Value.UInt8);
            hasValue = true;
            break;
        case jarvis::dt_e::UINT16:
            value = static_cast<int16_t>(datum.Value.UInt16);
            hasValue = true;
            break;
        case jarvis::dt_e::STRING:
            {
                const auto mappingRules = node.GetMappingRules();
                for (auto& pair : mappingRules)
                {
                    const std::string strValue = std::string(datum.Value.String.Data);
                    if (pair.second == strValue)
                    {
                        value = static_cast<int16_t>(pair.first);
                        hasValue = true;
                    }
                }
            }
            break;
        default:
            break;
        }

        if (hasValue == false)
        {
            LOG_ERROR(logger, "NO VALUE AVAILABLE FOR CONDITION TYPE ALARM");
            return;
        }
        

        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        std::string alarmUID;
        for (auto& node : nodeStore)
        {
            if (cin.GetNodeID().second == node.first)
            {
                alarmUID = node.second.GetUID();
                break;
            }
        }
        ASSERT((alarmUID.length() == 4), "THE LENGTH OF ALARM UID MUST BE 4");

        const bool isNewAlarm = isActiveAlarm(alarmUID) == false;
        bool isCondition = false;

        const std::vector<int16_t> vectorCondition = cin.GetCondition().second;
        for (auto& condition : vectorCondition)
        {
            if (value == condition)
            {
                isCondition = true;
                break;
            }
        }
        
        if (isNewAlarm == true)
        {
            if (isCondition == true)
            {
                activateAlarm(jarvis::alarm_type_e::CONDITION, cin, node);
            }
            else
            {
                return;
            }
        }
        else
        {
            if (isCondition == true)
            {
                return;
            }
            else
            {
                deactivateAlarm(jarvis::alarm_type_e::CONDITION, cin);
            }
        }
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

    float AlarmMonitor::convertToFloat(const im::var_data_t& datum)
    {
        ASSERT((datum.DataType != jarvis::dt_e::BOOLEAN), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF BOOLEAN DATA TYPE");
        ASSERT((datum.DataType != jarvis::dt_e::STRING), "LCL CANNOT BE APPLIED TO VARIABLE NODE OF STRING DATA TYPE");

        /**
         * @todo double 타입을 처리할 수 있도록 코드를 수정해야 합니다.
         */
        switch (datum.DataType)
        {
        case jarvis::dt_e::INT8:
            return static_cast<float>(datum.Value.Int8);
        case jarvis::dt_e::UINT8:
            return static_cast<float>(datum.Value.UInt8);
        case jarvis::dt_e::INT16:
            return static_cast<float>(datum.Value.Int16);
        case jarvis::dt_e::UINT16:
            return static_cast<float>(datum.Value.UInt16);
        case jarvis::dt_e::INT32:
            return static_cast<float>(datum.Value.Int32);
        case jarvis::dt_e::UINT32:
            return static_cast<float>(datum.Value.UInt32);
        case jarvis::dt_e::INT64:
            return static_cast<float>(datum.Value.Int64);
        case jarvis::dt_e::UINT64:
            return static_cast<float>(datum.Value.UInt64);
        case jarvis::dt_e::FLOAT32:
            return static_cast<float>(datum.Value.Float32);
        case jarvis::dt_e::FLOAT64:
            return static_cast<float>(datum.Value.Float64);
        default:
            return 0.0f;
        }
    }

    alarm_struct_t AlarmMonitor::retrieveActiveAlarm(const std::string& uid)
    {
        alarm_struct_t alarm;
        std::vector<muffin::alarm_struct_t>::iterator it;
        for (it = mVectorAlarmInfo.begin(); it != mVectorAlarmInfo.end(); ++it)
        {
            if (it->Uid == uid)
            {
                alarm = *it;
                mVectorAlarmInfo.erase(it);
                break;
            }
        }

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
    
    void AlarmMonitor::activateAlarm(const jarvis::alarm_type_e type, const jarvis::config::Alarm cin, const im::Variable& node)
    {
        alarm_struct_t alarm;
        push_struct_t push;
        push.SourceTimestamp = GetTimestampInMillis();
        push.Topic = mqtt::topic_e::PUSH;

        alarm.Topic = mqtt::topic_e::ALARM;
        alarm.AlarmType = "start";
        alarm.AlarmStartTime = GetTimestampInMillis();
        alarm.AlarmFinishTime = -1;

        switch (type)
        {
        case jarvis::alarm_type_e::ONLY_LCL:
            alarm.Uid = cin.GetLclAlarmUID().second;
            alarm.Name = node.GetDisplayName() + " 하한 도달";
            push.Name = node.GetDisplayName() + " 하한 도달";
            break;
        case jarvis::alarm_type_e::ONLY_UCL:
            alarm.Uid = cin.GetUclAlarmUID().second;
            alarm.Name = node.GetDisplayName() + " 상한 초과";
            push.Name = node.GetDisplayName() + " 상한 초과";
            break;
        case jarvis::alarm_type_e::CONDITION:
            {
                im::NodeStore& nodeStore = im::NodeStore::GetInstance();
                for (auto& nodeRef : nodeStore)
                {
                    if (cin.GetNodeID().second == nodeRef.first)
                    {
                        alarm.Uid = nodeRef.second.GetUID();
                        alarm.Name = std::string(node.RetrieveData().Value.String.Data);
                        push.Name = std::string(node.RetrieveData().Value.String.Data);
                        break;
                    }
                }
            }
            break;
        default:
            break;
        }
        alarm.UUID = createAlarmUUID();
        

        JSON json;
        const std::string payload = json.Serialize(alarm);
        const mqtt::topic_e topic = alarm.Uid.at(0) == 'A' ? mqtt::topic_e::ALARM : mqtt::topic_e::ERROR;
        mqtt::Message AlarmMessage(topic, payload);
        
        const std::string pushPayload = json.Serialize(push);
        const mqtt::topic_e pushTopic = push.Topic;
        mqtt::Message pushMessage(pushTopic, pushPayload);

        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        cdo.Store(AlarmMessage);
        cdo.Store(pushMessage);

        mVectorAlarmInfo.emplace_back(alarm);
    }

    void AlarmMonitor::deactivateAlarm(const jarvis::alarm_type_e type, const jarvis::config::Alarm cin)
    {
        std::string uid;

        switch (type)
        {
        case jarvis::alarm_type_e::ONLY_LCL:
            uid = cin.GetLclAlarmUID().second;
            break;
        case jarvis::alarm_type_e::ONLY_UCL:
            uid = cin.GetUclAlarmUID().second;
            break;
        case jarvis::alarm_type_e::CONDITION:
            {
                im::NodeStore& nodeStore = im::NodeStore::GetInstance();
                for (auto& node : nodeStore)
                {
                    if (cin.GetNodeID().second == node.first)
                    {
                        uid = node.second.GetUID();
                        break;
                    }
                }
            }
            break;
        default:
            break;
        }
        
        alarm_struct_t alarm = retrieveActiveAlarm(uid);

        alarm.AlarmFinishTime = GetTimestampInMillis();
        alarm.AlarmType = "finish";

        JSON json;
        const std::string payload = json.Serialize(alarm);
        const mqtt::topic_e topic = alarm.Uid.at(0) == 'A' ? mqtt::topic_e::ALARM : mqtt::topic_e::ERROR;
        mqtt::Message message(topic, payload);

        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        cdo.Store(message);
    }

    std::pair<bool,std::vector<std::string>> AlarmMonitor::GetUclUid()
    {
        std::vector<std::string> mVector;
        std::pair<muffin::Status, std::string> ret = std::make_pair(Status(Status::Code::UNCERTAIN),"");

        for (auto& cin : mVectorConfig)
        {
            ret = cin.GetUclUID();
            if (ret.first == Status::Code::GOOD)
            {
                mVector.emplace_back(ret.second);
            }
        }
        
        if (mVector.size() == 0)
        {
            return std::make_pair(false,mVector);
        }
        else
        {
            return std::make_pair(true,mVector);
        }

    }

    std::pair<bool,std::vector<std::string>> AlarmMonitor::GetLclUid()
    {
        std::vector<std::string> mVector;
        std::pair<muffin::Status, std::string> ret = std::make_pair(Status(Status::Code::UNCERTAIN),"");

        for (auto& cin : mVectorConfig)
        {
            ret = cin.GetLclUID();
            if (ret.first == Status::Code::GOOD)
            {
                mVector.emplace_back(ret.second);
            }
        }
        
        if (mVector.size() == 0)
        {
            return std::make_pair(false,mVector);
        }
        else
        {
            return std::make_pair(true,mVector);
        }

    }

    bool AlarmMonitor::ConvertUCL(std::string ucluid, std::string ucl)
    {
        bool decimalFound = false;
        size_t start = (ucl[0] == '-') ? 1 : 0;
        for (size_t i = start; i < ucl.length(); ++i) 
        {
            char c = ucl[i];
            if (c == '.') 
            {
                if (decimalFound) 
                {
                    return false;
                }
                decimalFound = true;
            } 
            else if (!isdigit(c)) 
            {
                return false;
            }
        }
        
        std::pair<muffin::Status, std::string> ret = std::make_pair(Status(Status::Code::UNCERTAIN),"");

        for (auto& cin : mVectorConfig)
        {
            ret = cin.GetUclUID();
            
            if (ret.first == Status::Code::GOOD)
            {
                if (ret.second == ucluid)
                {
                    float floatUCL = Convert.ToFloat(ucl);
                    cin.SetUCL(floatUCL);
                    updateFlashUclValue(cin.GetNodeID().second,floatUCL);
                    return true; 
                }
            }

        }

        return true;
    }

    bool AlarmMonitor::ConvertLCL(std::string lcluid, std::string lcl)
    {
        bool decimalFound = false;
        size_t start = (lcl[0] == '-') ? 1 : 0;
        for (size_t i = start; i < lcl.length(); ++i) 
        {
            char c = lcl[i];
            if (c == '.') 
            {
                if (decimalFound) 
                {
                    return false;
                }
                decimalFound = true;
            } 
            else if (!isdigit(c)) 
            {
                return false;
            }
        }
        
        std::pair<muffin::Status, std::string> ret = std::make_pair(Status(Status::Code::UNCERTAIN),"");

        for (auto& cin : mVectorConfig)
        {
            ret = cin.GetLclUID();
            if (ret.first == Status::Code::GOOD)
            {
                if (ret.second == lcluid)
                {
                    float floatLCL = Convert.ToFloat(lcl);
                    cin.SetLCL(floatLCL);
                    updateFlashLclValue(cin.GetNodeID().second,floatLCL);
                }
            }
        }
        
        return true;
    }

    void AlarmMonitor::updateFlashUclValue(std::string nodeid, float ucl)
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        fs::File file = esp32FS.Open("/jarvis/config.json");
        std::string payload;
        std::string resultPayload;
        while (file.available() > 0)
        {
            payload += file.read();
        }

        file.close();

        JSON json;
        JsonDocument doc;
        json.Deserialize(payload, &doc);
        JsonArray alarms = doc["cnt"]["alarm"];
        for (JsonObject alarm : alarms)
        {
            if (alarm["nodeId"].as<std::string>() == nodeid)
            {
                alarm["ucl"] = ucl;
                serializeJson(doc, resultPayload);
                file = esp32FS.Open("/jarvis/config.json", "w", true);
                for (size_t i = 0; i < resultPayload.length(); ++i)
                {
                    file.write(resultPayload[i]);
                }
                file.close();
            }
        }
    }

    void AlarmMonitor::updateFlashLclValue(std::string nodeid, float lcl)
    {
        ESP32FS& esp32FS = ESP32FS::GetInstance();
        fs::File file = esp32FS.Open("/jarvis/config.json");
        std::string payload;
        std::string resultPayload;
        while (file.available() > 0)
        {
            payload += file.read();
        }
        file.close();

        JSON json;
        JsonDocument doc;
        json.Deserialize(payload, &doc);
        JsonArray alarms = doc["cnt"]["alarm"];
        for (JsonObject alarm : alarms)
        {
            if (alarm["nodeId"].as<std::string>() == nodeid)
            {
                alarm["lcl"] = lcl;
                serializeJson(doc, resultPayload);

                file = esp32FS.Open("/jarvis/config.json", "w", true);
                for (size_t i = 0; i < resultPayload.length(); ++i)
                {
                    file.write(resultPayload[i]);
                }
                file.close();
            }
        }
    }

    void AlarmMonitor::pubLclToScautr(jarvis::config::Alarm& cin, const im::Variable& node)
    {
        float lcl = cin.GetLCL().second;
        if (lcl != cin.mPreviousLCL)
        {
            daq_struct_t param;

            param.SourceTimestamp = GetTimestampInMillis();
            param.Name = node.GetDisplayName() + " 하한 값";
            param.Uid = cin.GetLclUID().second;
            param.Unit = node.GetDisplayUnit();
            param.Value = node.FloatConvertToStringForLimitValue(lcl);

            JSON json;
            const std::string payload = json.Serialize(param);
            const mqtt::topic_e topic = mqtt::topic_e::DAQ_PARAM;
            mqtt::Message message(topic, payload);

            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            cdo.Store(message);

            cin.mPreviousLCL = lcl;
        }
        
    }

    void AlarmMonitor::pubUclToScautr(jarvis::config::Alarm& cin, const im::Variable& node)
    {
        float ucl = cin.GetUCL().second;
        if (ucl != cin.mPreviousUCL)
        {
            daq_struct_t param;

            param.SourceTimestamp = GetTimestampInMillis();
            param.Name = node.GetDisplayName() + " 상한 값";
            param.Uid = cin.GetUclUID().second;
            param.Unit = node.GetDisplayUnit();
            param.Value = node.FloatConvertToStringForLimitValue(ucl);

            JSON json;
            const std::string payload = json.Serialize(param);
            const mqtt::topic_e topic = mqtt::topic_e::DAQ_PARAM;
            mqtt::Message message(topic, payload);

            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            cdo.Store(message);

            cin.mPreviousUCL = ucl;
        }
    }

    AlarmMonitor* AlarmMonitor::mInstance = nullptr;
}