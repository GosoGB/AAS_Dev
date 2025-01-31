/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
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
        JARVIS_STATUS_REQUEST   = 3,
        JARVIS_STATUS_RESPONSE  = 4,
        DAQ_INPUT               = 5,
        DAQ_OUTPUT              = 6,
        DAQ_PARAM               = 7,
        ALARM                   = 8,
        ERROR                   = 9,
        PUSH                    = 10,
        OPERATION               = 11,
        UPTIME                  = 12,
        FINISHEDGOODS           = 13,
        FOTA_CONFIG             = 14,
        FOTA_UPDATE             = 15,
        FOTA_STATUS             = 16,
        REMOTE_CONTROL_REQUEST  = 17,
        REMOTE_CONTROL_RESPONSE = 18
    } topic_e;

    typedef enum class MqttQoSEnum
        : uint8_t
    {
        QoS_0 = 0,
        QoS_1 = 1,
        QoS_2 = 2
    } qos_e;
}}