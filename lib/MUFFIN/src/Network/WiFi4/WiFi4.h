/**
 * @file WiFi4.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-09-03
 * @version 1.0.0
 *  
 * @todo JARVIS 설계 문서에 WPA2 authentication method 속성을 추가로 정의해야 합니다.
 * 그러나 그에 앞서서 각 auth 모드에 따라 사용자로부터 추가로 입력을 받아야 하는 데이터가 
 * 어떤 게 있는지를 먼저 확인해야 합니다. WPA2 auth 방법에는 아래의 세 가지가 있습니다.
 *   - WPA2_AUTH_TLS, WPA2_AUTH_PEAP, WPA2_AUTH_TTLS
 * 
 * @todo Wi-Fi 클라이언트와 서버 간의 거리에 비례하는 왕복시간(round trip time, RTT)을 
 * 정밀 시간 측정(find time measurement, FTM; IEEE 802.11-2016) 방법으로 계산한 다음
 * 그 결과를 보고하는 기능을 사용하면 어떤 장애가 있을 때 효율적으로 처리가 가능할지도..?
 * 
 * @todo 현재는 Wi-Fi 클라이언트 모드만 염두에 두었기 때문에 향후 Wi-Fi AP 모드가 필요한
 * 경우에는 현재의 소스 코드를 변경해야 합니다. 다만 SRP 원칙 등을 고려했을 때, 클래스를
 * 분리하는 것이 좋을지 아니면 하나의 클래스 안에 구현할지에 대한 의사결정이 필요합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <WiFiGeneric.h>

#include "JARVIS/Config/Network/WiFi4.h"
#include "Network/INetwork.h"



namespace muffin {

    class WiFi4 : public INetwork
    {
    public:
        WiFi4();
        virtual ~WiFi4() override;
    private:
        wifi_mode_t mMode = WIFI_MODE_STA;
        const char* mModeString[5] = {
            "NULL", 
            "Station",
            "AP",
            "AP/STA",
            "MAX"
        };
    private:
        char mMacAddress[13];
        char mHostname[18];

    public:
        typedef enum class WiFiFiniteStateMachineEnum
            : int8_t
        {
            FAILED_TO_CONNECT_TO_AP        = -3,
            FAILED_TO_START_WIFI           = -2,
            FAILED_TO_CONFIGURE            = -1,
            NOT_INITIALIZED_YET            =  0,
            SUCCEDDED_TO_INITIALIZE        =  1,
            SUCCEDDED_TO_CONFIGURE         =  2,
            SUCCEDDED_TO_SCAN_AP_LIST      =  3,
            SUCCEDDED_TO_START_CLIENT      =  4,
            SUCCEDDED_TO_CONNECT_TO_AP     =  5,
            SUCCEDDED_TO_GET_IP_ADDRESS    =  6,
            WIFI_CLIENT_HAS_STOPPED        = 10,
            WIFI_CLIENT_HAS_DISCONNECTED   = 11,
            WIFI_CLIENT_AUTH_MODE_CHANGED  = 12,
            WIFI_CLIENT_LOST_IP_ADDRESS    = 13
        } state_e;
    public:
        virtual Status Init() override;
        virtual Status Config(jarvis::config::Base* config) override;
        virtual Status Connect() override;
        virtual Status Disconnect() override;
        virtual Status Reconnect() override;
        virtual bool IsConnected() const override;
        virtual IPAddress GetIPv4() const override;
        virtual std::pair<Status, size_t> TakeMutex() override;
        virtual Status ReleaseMutex() override;
        const char* GetMacAddress() const;
        state_e GetState() const;
    private:
        static void getArduinoWiFiEvent(arduino_event_id_t event);
    private:
        static state_e mState;
        jarvis::config::WiFi4 mConfig;
        bool mIsArduinoEventCallbackRegistered = false;
    };
}