/**
 * @file DeprecableEthernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet Handle 값 사용을 위해 임시로 복사해둔 ETH.h 파일 사본입니다.
 *
 * @date 2025-02-17
 * @version 1.2.6
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#if defined(MODLINK_T2) || defined(MODLINK_B)

#include <esp_eth.h>
#include <esp_eth_com.h>
#include <esp_eth_mac.h>
#include <esp_eth_phy.h>
#include <esp_event.h>
#include <esp_system.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <soc/emac_ext_struct.h>
#include <soc/rtc.h>

#include "Common/Logger/Logger.h"
#include "DeprecableEthernet.h"
#include "IM/Custom/MacAddress/MacAddress.h"

extern void tcpipInit();
extern void add_esp_interface_netif(esp_interface_t interface, esp_netif_t *esp_netif); /* from WiFiGeneric */


bool DeprecableEthernet::mIsTcpStackInitialized = false;



DeprecableEthernet::DeprecableEthernet()
    : initialized(false)
    , staticIP(false)
    , eth_handle(NULL)  // handle of Ethernet driver
    , started(false)
{
}

DeprecableEthernet::~DeprecableEthernet()
{
}

bool DeprecableEthernet::Begin()
{
    if (mIsTcpStackInitialized == false)
    {
        tcpipInit();
        mIsTcpStackInitialized = true;
    }

    esp_err_t ret = tcpip_adapter_set_default_eth_handlers();
    if (ret != ESP_OK)
    {
        LOG_ERROR(muffin::logger, "FAILED TO SET DEFAULT ETHERNET HANDLER: %u", ret);
        return false;
    }

    esp_netif_config_t cfg;
    cfg.base    = ESP_NETIF_BASE_DEFAULT_ETH;
    cfg.driver  = NULL;
    cfg.stack   = ESP_NETIF_NETSTACK_DEFAULT_ETH;

    mInterface = esp_netif_new(&cfg);
    if (mInterface == nullptr)
    {
        LOG_ERROR(muffin::logger, "FAILED TO CREATE ETHERNET INTERFACE INSTANCE");
        return false;
    }

    eth_mac_config_t macConfig;
    macConfig.sw_reset_timeout_ms = 1000;
    macConfig.rx_task_stack_size = 2048;
    macConfig.rx_task_prio = 15;
    macConfig.smi_mdc_gpio_num = 23;
    macConfig.smi_mdio_gpio_num = 18;
    macConfig.flags = 0;
    macConfig.interface = EMAC_DATA_INTERFACE_RMII;
    macConfig.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
    macConfig.clock_config.rmii.clock_gpio = EMAC_CLK_IN_GPIO;
    
    mEthernetMAC = esp_eth_mac_new_esp32(&macConfig);
    if (mEthernetMAC == nullptr)
    {
        LOG_ERROR(muffin::logger, "FAILED TO CREATE ETHERNET MAC INSTANCE");
        return false;
    }

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = mPhyAddress;
    phy_config.reset_gpio_num = mPhyPower;

    esp_eth_phy_t* eth_phy = esp_eth_phy_new_lan8720(&phy_config);
    if (eth_phy == nullptr)
    {
        LOG_ERROR(muffin::logger, "FAILED TO CREATE ETHERNET PHY INSTANCE");
        return false;
    }

    eth_handle = NULL;
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mEthernetMAC, eth_phy);
    if (esp_eth_driver_install(&eth_config, &eth_handle) != ESP_OK || eth_handle == NULL)
    {
        log_e("esp_eth_driver_install failed");
        return false;
    }

    /* attach Ethernet driver to TCP/IP stack */
    if (esp_netif_attach(mInterface, esp_eth_new_netif_glue(eth_handle)) != ESP_OK)
    {
        log_e("esp_netif_attach failed");
        return false;
    }

    /* attach to WiFiGeneric to receive events */
    add_esp_interface_netif(ESP_IF_ETH, mInterface);

    char hostName[32] = {'\0'};
#if defined(MODLINK_T2)
    snprintf(hostName, 32, "MODLINK-T2-%s", muffin::macAddress.GetEthernet());
#elif defined(MODLINK_B)
    snprintf(hostName, 32, "MODLINK-B-%s", muffin::macAddress.GetEthernet());
#elif defined(MT11)
    snprintf(hostName, 32, "MT11-%s", muffin::macAddress.GetEthernet());
#endif
    
    ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostName);
    if (ret != ESP_OK)
    {
        LOG_ERROR(muffin::logger, "FAILED TO SET HOSTNAME WITH CODE: %u", ret);
        std::abort();
    }
    LOG_INFO(muffin::logger, "Host name: %s", hostName);
    
    if (esp_eth_start(eth_handle) != ESP_OK)
    {
        log_e("esp_eth_start failed");
        return false;
    }
    
    // holds a few milliseconds to let DHCP start and enter into a good state
    // FIX ME -- adresses issue https://github.com/espressif/arduino-esp32/issues/5733
    delay(50);
    return true;
}

bool DeprecableEthernet::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
    esp_err_t err = ESP_OK;
    tcpip_adapter_ip_info_t info;

    if (static_cast<uint32_t>(local_ip) != 0)
    {
        info.ip.addr = static_cast<uint32_t>(local_ip);
        info.gw.addr = static_cast<uint32_t>(gateway);
        info.netmask.addr = static_cast<uint32_t>(subnet);
    }
    else
    {
        info.ip.addr = 0;
        info.gw.addr = 0;
        info.netmask.addr = 0;
    }

    err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
    if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED)
    {
        log_e("DHCP could not be stopped! Error: %d", err);
        return false;
    }

    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
    if (err != ERR_OK)
    {
        log_e("STA IP could not be configured! Error: %d", err);
        return false;
    }

    if (info.ip.addr)
    {
        staticIP = true;
    }
    else
    {
        err = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
        if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED)
        {
            log_w("DHCP could not be started! Error: %d", err);
            return false;
        }
        staticIP = false;
    }

    ip_addr_t d;
    d.type = IPADDR_TYPE_V4;

    if (static_cast<uint32_t>(dns1) != 0)
    {
        // Set DNS1-Server
        d.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
        dns_setserver(0, &d);
    }

    if (static_cast<uint32_t>(dns2) != 0)
    {
        // Set DNS2-Server
        d.u_addr.ip4.addr = static_cast<uint32_t>(dns2);
        dns_setserver(1, &d);
    }

    return true;
}

IPAddress DeprecableEthernet::GetIPv4()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.ip.addr);
}

IPAddress DeprecableEthernet::GetSubnetMask()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.netmask.addr);
}

IPAddress DeprecableEthernet::GetGateway()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.gw.addr);
}

IPAddress DeprecableEthernet::GetDNS1()
{
    const ip_addr_t *dns_ip = dns_getserver(0);
    return IPAddress(dns_ip->u_addr.ip4.addr);
}

IPAddress DeprecableEthernet::GetDNS2()
{
    const ip_addr_t *dns_ip = dns_getserver(1);
    return IPAddress(dns_ip->u_addr.ip4.addr);
}

IPAddress DeprecableEthernet::GetBroadcast()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

IPAddress DeprecableEthernet::GetNetworkID()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

uint8_t DeprecableEthernet::GetSubnetCIDR()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return (uint8_t)0;
    }
    return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip.netmask.addr));
}

bool DeprecableEthernet::fullDuplex()
{
    eth_duplex_t link_duplex;
    esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, &link_duplex);
    return (link_duplex == ETH_DUPLEX_FULL);
}

bool DeprecableEthernet::linkUp()
{
    return WiFiGenericClass::getStatusBits() & ETH_CONNECTED_BIT;
}

uint8_t DeprecableEthernet::linkSpeed()
{
    eth_speed_t link_speed;
    esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
    return (link_speed == ETH_SPEED_10M) ? 10 : 100;
}


DeprecableEthernet deprecableEthernet;

#endif