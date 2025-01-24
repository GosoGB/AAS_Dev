/**
 * @file JarvisTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <ArduinoJson.h>

#include "JARVIS/Include/Base.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Validators/ValidationResult.h"
#include "Protocol/MQTT/Include/Message.h"



namespace muffin {

    typedef struct JarvisTaskParameters
    {
        void (*Callback)(muffin::jvs::ValidationResult&);
    } jarvis_task_params;


    void ProcessJarvisRequestTask(void* pvParameters);
    void RetrieveJarvisRequestPayload(std::string* outputpayload);
    void ApplyJarvisTask();

    void applyAlarmCIN(std::vector<jvs::config::Base*>& vectorAlarmCIN);
    void applyNodeCIN(std::vector<jvs::config::Base*>& vectorNodeCIN);
    void applyOperationTimeCIN(std::vector<jvs::config::Base*>& vectorOperationTimeCIN);
    void applyProductionInfoCIN(std::vector<jvs::config::Base*>& vectorProductionInfoCIN);
    void applyRS485CIN(std::vector<jvs::config::Base*>& vectorRS485CIN);

    void applyModbusRtuCIN(std::vector<jvs::config::Base*>& vectorModbusRTUCIN, jvs::config::Rs485* rs485CIN);
    void applyModbusTcpCIN(std::vector<jvs::config::Base*>& vectorModbusTCPCIN);
    
    Status strategyCatHttp();
    Status strategyLwipHttp();
}