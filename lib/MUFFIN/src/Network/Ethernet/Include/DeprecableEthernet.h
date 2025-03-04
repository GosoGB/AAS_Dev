/**
 * @file DeprecableEthernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet Handle 값 사용을 위해 임시로 복사해둔 ETH.h 파일 사본입니다.
 *
 * @date 2025-02-17
 * @version 1.2.6
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo 만약 IPv6 표준이 필요한 경우 IPv6Address 클래스를 사용하여 개발할 것
 */




#if defined(MODLINK_T2) || defined(MODLINK_B)

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