#if defined(MT10) || defined(MB10)

/**
 * @file LAN8720.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LAN8720 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @todo Network 모듈 단에서 LAN8720 인터페이스의 MAC 주소를 어떻게 읽어가는
 * 것이 좋을지 결정하지 못하였습니다. 현재는 다음의 두 개의 방안을 고민 중입니다.
 *   - 방법 #1: GetMacAddress() 함수를 정적 메서드로 선언
 *   - 방법 #2: LAN8720() -> Init() -> GetMacAddress() 순서로 호출
 * 
 * @todo Arduino 프레임워크의 제약으로 인해 Disconnect(), Reconnect() 함수는 
 * 개발하지 않기로 결정하였습니다. 향후에 ESP-IDF 프레임워크로 이전하게 된다면
 * Disconnect(), Reconnect() 함수를 개발해야 합니다.
 * 
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */



 
#pragma once

#include <WiFiGeneric.h>

#include "Common/DataStructure/bitset.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "Network/INetwork.h"



namespace muffin {
    class LAN8720 : public INetwork
    {
    public:
        LAN8720();
        virtual ~LAN8720() override {}
    public:
        virtual Status Init() override;
        virtual Status Config(jvs::config::Base* config) override;
        virtual Status Connect() override;
        virtual Status Disconnect() override;
        virtual Status Reconnect() override;
        virtual bool IsConnected() override;
        virtual IPAddress GetIPv4() const override;
        virtual Status SyncNTP() override;
        virtual std::pair<Status, size_t> TakeMutex() override;
        virtual Status ReleaseMutex() override;
        virtual bool IsIPv4Assigned() override;
    public:
        IPAddress GetSubnetMask();
        IPAddress GetGateway();
        IPAddress GetDNS1();
        IPAddress GetDNS2();
        IPAddress GetBroadcast();
        IPAddress GetNetworkID();
        uint8_t GetSubnetCIDR();
        uint8_t GetlinkSpeed();
        bool IsFullDuplex();
        bool IsLinkUp();
    private:
        bool begin();
        bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2);
        static void getArduinoEthernetEvent(arduino_event_id_t event);
        void implArduinoCallback(arduino_event_id_t event);
    private:
        typedef enum class InitFlagEnum
            : uint8_t
        {
            INITIALIZED  = 0,
            CONFIGURED   = 1,
            HAS_STARTED  = 2,
            HAS_IPv4     = 3,
            TOP          = 4
        } flag_e;
        bitset<static_cast<uint8_t>(flag_e::TOP)> mFlags;
        bool mIsTcpStackInitialized = false;
        esp_netif_t* mInterface = nullptr;
        esp_eth_mac_t* mEthernetMAC = nullptr;
typedef enum
{
    ETH_CLOCK_GPIO0_IN,
    ETH_CLOCK_GPIO0_OUT,
    ETH_CLOCK_GPIO16_OUT,
    ETH_CLOCK_GPIO17_OUT
} eth_clock_mode_t;
        const eth_clock_mode_t mClockMode = ETH_CLOCK_GPIO0_IN;
        const uint8_t mPhyAddress = 1;
        const uint8_t mPhyPower = 32;
    esp_eth_handle_t eth_handle = NULL;  // handle of Ethernet driver
    bool staticIP = false;
    };


    extern LAN8720* ethernet;
}


#endif