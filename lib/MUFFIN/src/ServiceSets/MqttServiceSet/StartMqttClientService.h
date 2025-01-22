/**
 * @file StartMqttClientService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 클라이언트 초기화 및 연결하는 서비스를 선언합니다.
 * 
 * @date 2025-01-22
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once



namespace muffin {

    Status InitializeMqttClient();
    Status Connect2Broker();
}