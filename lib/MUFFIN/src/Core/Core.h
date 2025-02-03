/**
 * @file Core.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크를 초기화 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2025-01-25
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * 
 * @todo 사용자로부터 네트워크 설정 정보를 받는 기능을 구현해야 합니다.
 *       네트워크 설정 변경과 같은 사유로 인해 인터넷에 연결될 수 없는 상황일 때,
 *       사용자로부터 네트워크 설정을 받아 적용하는 프로세스를 만들어야 합니다.
 */




#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Status.h"
#include "IM/Custom/TypeDefinitions.h"
#include "Protocol/MQTT/Include/Message.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "JARVIS/Config/Protocol/ModbusTCP.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "JARVIS/Validators/ValidationResult.h"



namespace muffin {
    
    extern std::vector<muffin::jvs::config::ModbusRTU> mVectorModbusRTU;
    extern std::vector<muffin::jvs::config::ModbusTCP> mVectorModbusTCP;
    extern muffin::jvs::config::Ethernet mEthernet; 
    
    
    class Core
    {
    public:
        void Init();
    private:
        Status readInitConfig(init_cfg_t* output);
        static Status writeInitConfig(const init_cfg_t& config);
        bool isResetByPanic();
        Status createDefaultJARVIS();
        Status loadJarvisConfig();
    };


    extern Core core;
}