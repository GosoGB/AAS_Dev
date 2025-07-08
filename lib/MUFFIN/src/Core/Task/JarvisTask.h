/**
 * @file JarvisTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 선언합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <ArduinoJson.h>

#include "JARVIS/Include/Base.h"
#include "JARVIS/Config/Interfaces/Rs485.h"
#include "JARVIS/Validators/ValidationResult.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/Melsec/MelsecClient.h"
#include "Protocol/EthernetIP/ciplibs/eip_session.h"


namespace muffin {

    void ApplyJarvisTask();

    void applyAlarmCIN(std::vector<jvs::config::Base*>& vectorAlarmCIN);
    void applyNodeCIN(std::vector<jvs::config::Base*>& vectorNodeCIN);
    void applyOperationTimeCIN(std::vector<jvs::config::Base*>& vectorOperationTimeCIN);
    void applyProductionInfoCIN(std::vector<jvs::config::Base*>& vectorProductionInfoCIN);
    void applyRS485CIN(std::vector<jvs::config::Base*>& vectorRS485CIN);

    void applyModbusRtuCIN(std::vector<jvs::config::Base*>& vectorModbusRTUCIN, jvs::config::Rs485* rs485CIN);
    void applyModbusTcpCIN(std::vector<jvs::config::Base*>& vectorModbusTCPCIN);
    void applyMelsecCIN(std::vector<jvs::config::Base*>& vectorMelsecCIN);
    void applyEthernet();
#if defined(MT11)
    void applyModbusTcpConfig(W5500* eth, w5500::sock_id_e id, jvs::config::ModbusTCP* cin);
    void applyMelsecConfig(MelsecClient* client, jvs::config::Melsec* cin);
    void applayEthernetIpCIN(std::vector<jvs::config::Base*>& vectorEthernetIpCIN);
    void applyEthernetIpConfig(EIPSession eipSession, jvs::config::EthernetIP* cin);
#endif

}