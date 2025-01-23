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
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/INetwork.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"



namespace muffin {

    mqtt::BrokerInfo brokerInfo(
        macAddress.GetEthernet(),           // clientID
        "mqtt.vitcon.iotops.opsnow.com",    // host
        8883,                               // port
        7,                                  // keepalive
        mqtt::socket_e::SOCKET_0,           // socketID
        "vitcon",                           // username
        "tkfkdgo5!@#$"                      // password
    );

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

    Status strategyInitCatM1()
    {
        ;
    }

    Status strategyInitEthernet()
    {
        mqtt::Message lwt = GenerateWillMessage(false);
        mqtt::LwipMQTT* lwipMQTT = new mqtt::LwipMQTT(brokerInfo, lwt);

        Status ret = lwipMQTT->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE LwIP MQTT Client: %s", ret.c_str());
            return ret;
        }
        mqttClient = lwipMQTT;

        ;
        
    /*
        http::LwipHTTP* lwipHTTP = new http::LwipHTTP();
        ret = lwipHTTP->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE LwIP HTTTP Client: %s", ret.c_str());
            return;
        }
        httpClient = lwipHTTP;
        break;
    */
    }

    Status strategyInitCatM1()
    {
        ;
    }


    Status InitMqttService()
    {
        switch (jvs::config::operationCIN.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            return ;

        case jvs::snic_e::Ethernet:
            return strategyInitEthernet();
        
        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operationCIN.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status ConnectMqttService()
    {

        mqtt::Message lwt = GenerateWillMessage(false);
        
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}