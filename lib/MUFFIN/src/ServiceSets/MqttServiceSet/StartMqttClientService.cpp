/**
 * @file StartMqttClientService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 클라이언트 초기화 및 연결하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Network/INetwork.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Message.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"



namespace muffin {

    mqtt::Message GenerateWillMessage(const bool isConnected)
    {
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        snprintf(buffer, size, "%s,%llu,%s,%s,null",
            macAddress.GetEthernet(),
            GetTimestampInMillis(),
            isConnected ? "true" : "false",
            FW_VERSION_ESP32.GetSemanticVersion()
        );
    #else
        snprintf(buffer, size, "%s,%llu,%s,%s,%s",
            macAddress.GetEthernet(),
            GetTimestampInMillis(),
            isConnected ? "true" : "false",
            FW_VERSION_ESP32.GetSemanticVersion(),
            FW_VERSION_MEGA2560.GetSemanticVersion()
        );
    #endif
        
        return mqtt::Message(mqtt::topic_e::LAST_WILL, buffer);
    }


    Status InitializeMqttClient()
    {
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status Connect2Broker()
    {
        mqtt::Message lwt = GenerateWillMessage(false);
        
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }



}