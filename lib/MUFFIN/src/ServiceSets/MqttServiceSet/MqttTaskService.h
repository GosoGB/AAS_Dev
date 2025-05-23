/**
 * @file MqttTaskService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 태스크를 실행하고 정지하는 서비스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "IM/Custom/TypeDefinitions.h"



namespace muffin {
    
    Status PublishResponseJARVIS(const jarvis_struct_t& response);
    Status StartMqttTaskService(init_cfg_t& config, CallbackUpdateInitConfig callbackJARVIS);
    Status StopMqttTaskService();
    Status RemoteControllToMachine(remote_controll_struct_t* message, JsonArray& mc);
    Status RemoteControllToModlink(remote_controll_struct_t* message, JsonArray& md);
}