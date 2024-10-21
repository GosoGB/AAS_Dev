/**
 * @file JarvisTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <ArduinoJson.h>

#include "Protocol/MQTT/Include/Message.h"



namespace muffin {

    void ProcessJarvisRequestTask(void* pvParameters);
    void RetrieveJarvisRequestPayload(std::string* outputpayload);
    void ApplyJarvisTask();

    void applyAlarmCIN(std::vector<jarvis::config::Base*>& vectorAlarmCIN);
    void applyNodeCIN(std::vector<jarvis::config::Base*>& vectorNodeCIN);
    void applyOperationTimeCIN(std::vector<jarvis::config::Base*>& vectorOperationTimeCIN);
    void applyProductionInfoCIN(std::vector<jarvis::config::Base*>& vectorProductionInfoCIN);
    void applyRS485CIN(std::vector<jarvis::config::Base*>& vectorRS485CIN);
    
    void applyLteCatM1CIN(std::vector<jarvis::config::Base*>& vectorLteCatM1CIN);
    // void applyEthernetCIN(std::vector<jarvis::config::Base*>& vectorEthernetCIN);
    // void applyWiFi4CIN(std::vector<jarvis::config::Base*>& vectorWiFi4CIN);

    void applyOperationCIN(std::vector<jarvis::config::Base*>& vectorOperationCIN);
    void applyModbusRtuCIN(std::vector<jarvis::config::Base*>& vectorModbusRTUCIN, std::vector<jarvis::config::Base*>& vectorRS485CIN);
    // void applyModbusTcpCIN(std::vector<jarvis::config::Base*>& vectorModbusTCPCIN);
}