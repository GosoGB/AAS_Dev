/**
 * @file DeprecableEthernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet Handle 값 사용을 위해 임시로 복사해둔 ETH.h 파일 사본입니다.
 *
 * @date 2025-02-14
 * @version 1.2.6
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#if defined(MODLINK_T2) || defined(MODLINK_B)

#include "DeprecableEthernet.h"
#include "esp_system.h"
#if ESP_IDF_VERSION_MAJOR > 3
#include "esp_event.h"
#include "esp_eth.h"
#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "esp_eth_com.h"
#if CONFIG_IDF_TARGET_ESP32
#include "soc/emac_ext_struct.h"
#include "soc/rtc.h"
// #include "soc/io_mux_reg.h"
// #include "hal/gpio_hal.h"
#endif
#else
#include "eth_phy/phy.h"
#include "eth_phy/phy_tlk110.h"
#include "eth_phy/phy_lan8720.h"
#endif
#include "lwip/err.h"
#include "lwip/dns.h"

extern void tcpipInit();
extern void add_esp_interface_netif(esp_interface_t interface, esp_netif_t *esp_netif); /* from WiFiGeneric */

#if ESP_IDF_VERSION_MAJOR > 3
/**
 * @brief Callback function invoked when lowlevel initialization is finished
 *
 * @param[in] eth_handle: handle of Ethernet driver
 *
 * @return
 *       - ESP_OK: process extra lowlevel initialization successfully
 *       - ESP_FAIL: error occurred when processing extra lowlevel initialization
 */

static eth_clock_mode_t eth_clock_mode = ETH_CLK_MODE;

/**
 * @brief Callback function invoked when lowlevel deinitialization is finished
 *
 * @param[in] eth_handle: handle of Ethernet driver
 *
 * @return
 *       - ESP_OK: process extra lowlevel deinitialization successfully
 *       - ESP_FAIL: error occurred when processing extra lowlevel deinitialization
 */
#else
#endif

DeprecableEthernet::DeprecableEthernet()
    : initialized(false), staticIP(false)
#if ESP_IDF_VERSION_MAJOR > 3
      ,
      eth_handle(NULL)
#endif
      ,
      started(false)
{
}

DeprecableEthernet::~DeprecableEthernet()
{
}

bool DeprecableEthernet::begin(uint8_t phy_addr, int power, int mdc, int mdio, eth_phy_type_t type, eth_clock_mode_t clock_mode, bool use_mac_from_efuse)
{
#if ESP_IDF_VERSION_MAJOR > 3
    eth_clock_mode = clock_mode;
    tcpipInit();

    if (use_mac_from_efuse)
    {
        uint8_t p[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        esp_efuse_mac_get_custom(p);
        esp_base_mac_addr_set(p);
    }

    tcpip_adapter_set_default_eth_handlers();

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

    esp_eth_mac_t *eth_mac = NULL;
#if CONFIG_ETH_SPI_ETHERNET_DM9051
    if (type == ETH_PHY_DM9051)
    {
        return false; // todo
    }
    else
    {
#endif
#if CONFIG_ETH_USE_ESP32_EMAC
        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
        mac_config.clock_config.rmii.clock_mode = (eth_clock_mode) ? EMAC_CLK_OUT : EMAC_CLK_EXT_IN;
        mac_config.clock_config.rmii.clock_gpio = (1 == eth_clock_mode) ? EMAC_APPL_CLK_OUT_GPIO : (2 == eth_clock_mode) ? EMAC_CLK_OUT_GPIO
                                                                                               : (3 == eth_clock_mode)   ? EMAC_CLK_OUT_180_GPIO
                                                                                                                         : EMAC_CLK_IN_GPIO;
        mac_config.smi_mdc_gpio_num = mdc;
        mac_config.smi_mdio_gpio_num = mdio;
        mac_config.sw_reset_timeout_ms = 1000;
        eth_mac = esp_eth_mac_new_esp32(&mac_config);
#endif
#if CONFIG_ETH_SPI_ETHERNET_DM9051
    }
#endif

    if (eth_mac == NULL)
    {
        log_e("esp_eth_mac_new_esp32 failed");
        return false;
    }

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = phy_addr;
    phy_config.reset_gpio_num = power;
    esp_eth_phy_t *eth_phy = NULL;
    switch (type)
    {
    case ETH_PHY_LAN8720:
        eth_phy = esp_eth_phy_new_lan8720(&phy_config);
        break;
    case ETH_PHY_TLK110:
        eth_phy = esp_eth_phy_new_ip101(&phy_config);
        break;
    case ETH_PHY_RTL8201:
        eth_phy = esp_eth_phy_new_rtl8201(&phy_config);
        break;
    case ETH_PHY_DP83848:
        eth_phy = esp_eth_phy_new_dp83848(&phy_config);
        break;
#if CONFIG_ETH_SPI_ETHERNET_DM9051
    case ETH_PHY_DM9051:
        eth_phy = esp_eth_phy_new_dm9051(&phy_config);
        break;
#endif
    case ETH_PHY_KSZ8041:
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
        eth_phy = esp_eth_phy_new_ksz8041(&phy_config);
#else
        log_e("unsupported ethernet type 'ETH_PHY_KSZ8041'");
#endif
        break;
    case ETH_PHY_KSZ8081:
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
        eth_phy = esp_eth_phy_new_ksz8081(&phy_config);
#else
        log_e("unsupported ethernet type 'ETH_PHY_KSZ8081'");
#endif
        break;
    default:
        break;
    }
    if (eth_phy == NULL)
    {
        log_e("esp_eth_phy_new failed");
        return false;
    }

    eth_handle = NULL;
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(eth_mac, eth_phy);
    if (esp_eth_driver_install(&eth_config, &eth_handle) != ESP_OK || eth_handle == NULL)
    {
        log_e("esp_eth_driver_install failed");
        return false;
    }

    /* attach Ethernet driver to TCP/IP stack */
    if (esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)) != ESP_OK)
    {
        log_e("esp_netif_attach failed");
        return false;
    }

    /* attach to WiFiGeneric to receive events */
    add_esp_interface_netif(ESP_IF_ETH, eth_netif);

    if (esp_eth_start(eth_handle) != ESP_OK)
    {
        log_e("esp_eth_start failed");
        return false;
    }
#endif
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

IPAddress DeprecableEthernet::localIP()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.ip.addr);
}

IPAddress DeprecableEthernet::subnetMask()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.netmask.addr);
}

IPAddress DeprecableEthernet::gatewayIP()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.gw.addr);
}

IPAddress DeprecableEthernet::dnsIP(uint8_t dns_no)
{
    const ip_addr_t *dns_ip = dns_getserver(dns_no);
    return IPAddress(dns_ip->u_addr.ip4.addr);
}

IPAddress DeprecableEthernet::broadcastIP()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

IPAddress DeprecableEthernet::networkID()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

uint8_t DeprecableEthernet::subnetCIDR()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return (uint8_t)0;
    }
    return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip.netmask.addr));
}

const char *DeprecableEthernet::getHostname()
{
    const char *hostname;
    if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_ETH, &hostname))
    {
        return NULL;
    }
    return hostname;
}

bool DeprecableEthernet::setHostname(const char *hostname)
{
    return tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostname) == 0;
}

bool DeprecableEthernet::fullDuplex()
{
#if ESP_IDF_VERSION_MAJOR > 3
    eth_duplex_t link_duplex;
    esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, &link_duplex);
    return (link_duplex == ETH_DUPLEX_FULL);
#else
    return eth_config.phy_get_duplex_mode();
#endif
}

bool DeprecableEthernet::linkUp()
{
#if ESP_IDF_VERSION_MAJOR > 3
    return WiFiGenericClass::getStatusBits() & ETH_CONNECTED_BIT;
#else
    return eth_config.phy_check_link();
#endif
}

uint8_t DeprecableEthernet::linkSpeed()
{
#if ESP_IDF_VERSION_MAJOR > 3
    eth_speed_t link_speed;
    esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
    return (link_speed == ETH_SPEED_10M) ? 10 : 100;
#else
    return eth_config.phy_get_speed_mode() ? 100 : 10;
#endif
}

bool DeprecableEthernet::enableIpV6()
{
    return tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_ETH) == 0;
}

IPv6Address DeprecableEthernet::localIPv6()
{
    static ip6_addr_t addr;
    if (tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_ETH, &addr))
    {
        return IPv6Address();
    }
    return IPv6Address(addr.addr);
}

uint8_t *DeprecableEthernet::macAddress(uint8_t *mac)
{
    if (!mac)
    {
        return NULL;
    }
#ifdef ESP_IDF_VERSION_MAJOR
    esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac);
#else
    esp_eth_get_mac(mac);
#endif
    return mac;
}

String DeprecableEthernet::macAddress(void)
{
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
    char macStr[18] = {0};
    macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

DeprecableEthernet deprecableEthernet;

#endif