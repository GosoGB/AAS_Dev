/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin { namespace mqtt {

    typedef enum class MqttVersionEnum
        : uint8_t
    {
        UNDEFINED = 0,
        Ver_3_1_0 = 3,
        Ver_3_1_1 = 4
    } version_e;

    typedef enum class MqttSocketEnum
        : uint8_t
    {
        SOCKET_0 = 0,
        SOCKET_1 = 1,
        SOCKET_2 = 2,
        SOCKET_3 = 3,
        SOCKET_4 = 4,
        SOCKET_5 = 5
    } socket_e;

    typedef enum class topic_e
        : uint8_t
    {
        LAST_WILL               = 0,
        JARVIS_REQUEST          = 1,
        JARVIS_RESPONSE         = 2,
        REMOTE_CONTROL_REQUEST  = 3,
        REMOTE_CONTROL_RESPONSE = 4,
        DAQ_INPUT               = 5,
        DAQ_OUTPUT              = 6,
        DAQ_PARAM               = 7,
        ALARM                   = 8,
        ERROR                   = 9,
        OPERATION               = 10,
        UPTIME                  = 11,
        FINISHEDGOODS           = 12,
        FOTA_CONFIG             = 13,
        FOTA_UPDATE             = 14,
        FOTA_STATUS             = 15
    } topic_e;

    typedef enum class MqttQoSEnum
        : uint8_t
    {
        QoS_0 = 0,
        QoS_1 = 1,
        QoS_2 = 2
    } qos_e;
}}