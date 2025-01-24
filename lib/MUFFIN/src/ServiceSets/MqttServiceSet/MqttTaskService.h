/**
 * @file MqttTaskService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 태스크를 실행하고 정지하는 서비스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"



namespace muffin {
    
    Status StartMqttTaskService();
    Status StopMqttTaskService();
}