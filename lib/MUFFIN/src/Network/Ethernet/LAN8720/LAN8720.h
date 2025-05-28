#if defined(MT10) || defined(MB10)

/**
 * @file LAN8720.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @todo Network 모듈 단에서 Ethernet 인터페이스의 MAC 주소를 어떻게 읽어가는
 * 것이 좋을지 결정하지 못하였습니다. 현재는 다음의 두 개의 방안을 고민 중입니다.
 *   - 방법 #1: GetMacAddress() 함수를 정적 메서드로 선언
 *   - 방법 #2: Ethernet() -> Init() -> GetMacAddress() 순서로 호출
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

#include "WiFi.h"
#include "esp_system.h"
#include "esp_eth.h"

typedef enum
{
    ETH_CLOCK_GPIO0_IN,
    ETH_CLOCK_GPIO0_OUT,
    ETH_CLOCK_GPIO16_OUT,
    ETH_CLOCK_GPIO17_OUT
} eth_clock_mode_t;



class DeprecableEthernet
{
    friend class WiFiClient;
    friend class WiFiServer;

private:
    static bool mIsTcpStackInitialized;
    bool initialized;
    bool staticIP;
private:
    esp_netif_t* mInterface = nullptr;
    esp_eth_mac_t* mEthernetMAC = nullptr;

public:
    esp_eth_handle_t eth_handle;

protected:
    bool started;
    static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

public:
    DeprecableEthernet();
    ~DeprecableEthernet();
private:
    const eth_clock_mode_t mClockMode = ETH_CLOCK_GPIO0_IN;
    const uint8_t mPhyAddress = 1;
    const uint8_t mPhyPower = 32;

public:
    bool Begin();
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);

    bool fullDuplex();
    bool linkUp();
    uint8_t linkSpeed();

    IPAddress GetIPv4();
    IPAddress GetSubnetMask();
    IPAddress GetGateway();
    IPAddress GetDNS1();
    IPAddress GetDNS2();
    IPAddress GetBroadcast();
    IPAddress GetNetworkID();
    uint8_t GetSubnetCIDR();
};


extern DeprecableEthernet deprecableEthernet;
#endif