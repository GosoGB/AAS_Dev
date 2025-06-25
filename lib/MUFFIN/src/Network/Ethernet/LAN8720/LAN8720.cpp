#if defined(MT10) || defined(MB10)

/**
 * @file LAN8720.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 *
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <esp32-hal.h>
#include <lwip/dns.h>
#include <WiFi.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "LAN8720.h"
#include "Network/Ethernet/EthernetFactory.h"


extern void tcpipInit();
extern void add_esp_interface_netif(esp_interface_t interface, esp_netif_t *esp_netif); /* from WiFiGeneric */



namespace muffin {

    LAN8720::LAN8720()
    {
        mFlags.reset();
    }


    Status LAN8720::Init()
    {
        if (mFlags.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true)
        {
            return Status(Status::Code::GOOD);
        }

        if (WiFi.onEvent(getArduinoEthernetEvent) == 0)
        {
            LOG_ERROR(logger, "FAILED TO REGISTER CALLBACK");
            return Status(Status::Code::BAD_INTERNAL_ERROR);
        }
        LOG_VERBOSE(logger, "Registered Ethernet event callback");

        LOG_INFO(logger, "Initialized Ethernet interface");
        mFlags.set(static_cast<uint8_t>(flag_e::INITIALIZED));
        return Status(Status::Code::GOOD);
    }


    Status LAN8720::Config(jvs::config::Base* config)
    {
        ASSERT((config != nullptr), "INPUT PARAM <jvs::config::Base* config> CANNOT BE NULL");
        ASSERT((config->GetCategory() == jvs::cfg_key_e::ETHERNET), "INVALID JARVIS CATEGORY");
        ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true), "MUST BE INITIALIZED FIRST");
        mFlags.set(static_cast<uint8_t>(flag_e::CONFIGURED));
        return Status(Status::Code::GOOD);
    }


    Status LAN8720::Connect()
    {
        ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true), "MUST BE INITIALIZED FIRST");
        ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::CONFIGURED))  == true), "MUST BE CONFIGURED FIRST");

        if (mFlags.test(static_cast<uint8_t>(flag_e::HAS_STARTED)) == true)
        {
            return Status(Status::Code::GOOD);
        }
        
        if (begin() == false)
        {
            goto ON_FAIL;
        }

        if (jvs::config::embeddedEthernet->GetDHCP().second == false)
        {
            const bool isConfigured = config(jvs::config::embeddedEthernet->GetStaticIPv4().second,
                                                                jvs::config::embeddedEthernet->GetGateway().second,
                                                                jvs::config::embeddedEthernet->GetSubnetmask().second,
                                                                jvs::config::embeddedEthernet->GetDNS1().second,
                                                                jvs::config::embeddedEthernet->GetDNS2().second);

            if (isConfigured == false)
            {
                goto ON_FAIL;
            }
        }

        mFlags.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
        return Status(Status::Code::GOOD);

    ON_FAIL:
        LOG_ERROR(logger, "FAILED TO START ETHERNET PHY");
        return Status(Status::Code::BAD_DEVICE_FAILURE);
    }


    Status LAN8720::Disconnect()
    {
        LOG_ERROR(logger, "DISCONNECT IS NOT SUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }


    Status LAN8720::Reconnect()
    {
        LOG_ERROR(logger, "RECONNECT IS NOT SUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }


    bool LAN8720::IsConnected()
    {
        if (mFlags.test(static_cast<uint8_t>(flag_e::HAS_IPv4)) == true)
        {
            return true;
        }
        else
        {
            return false;
        }
    }


    IPAddress LAN8720::GetIPv4() const
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE IPv4 ADDRESS");
            return IPAddress();
        }
        return IPAddress(ip.ip.addr);
    }


    Status LAN8720::SyncNTP()
    {
        const char* ntpServer2 = "time.windows.com";
        const long gmtOffset_sec = 32400;
        const int daylightOffset_sec = 0;

        if (ntpServer == "time.google.com")
        {
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer.c_str(), ntpServer2);
        }
        else 
        {
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer.c_str(), ntpServer.c_str());
        }
        
        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            char buffer[11] = {'\0'};
            snprintf(buffer, sizeof(buffer), "%ld", GetTimestamp());
            
            if (strlen(buffer) == 10)
            {
                return Status(Status::Code::GOOD);
            }

            LOG_WARNING(logger, "[TRIAL: #%u] NTP SYNC WAS UNSUCCESSFUL: %s", trialCount, buffer);
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }
        
        LOG_ERROR(logger, "FAILED TO SYNC WITH NTP SERVER");
        return Status(Status::Code::BAD_INVALID_TIMESTAMP);
    }


    std::pair<Status, size_t> LAN8720::TakeMutex()
    {
        return std::make_pair(Status(Status::Code::GOOD), 1);
    }

    
    Status LAN8720::ReleaseMutex()
    {
        return Status(Status::Code::GOOD);
    }

    
    IPAddress LAN8720::GetSubnetMask()
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            return IPAddress();
        }
        return IPAddress(ip.netmask.addr);
    }


    IPAddress LAN8720::GetGateway()
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            return IPAddress();
        }
        return IPAddress(ip.gw.addr);
    }


    IPAddress LAN8720::GetDNS1()
    {
        const ip_addr_t *dns_ip = dns_getserver(0);
        return IPAddress(dns_ip->u_addr.ip4.addr);
    }


    IPAddress LAN8720::GetDNS2()
    {
        const ip_addr_t *dns_ip = dns_getserver(1);
        return IPAddress(dns_ip->u_addr.ip4.addr);
    }


    IPAddress LAN8720::GetBroadcast()
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            return IPAddress();
        }
        return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
    }


    IPAddress LAN8720::GetNetworkID()
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            return IPAddress();
        }
        return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
    }


    uint8_t LAN8720::GetSubnetCIDR()
    {
        tcpip_adapter_ip_info_t ip;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
        {
            return (uint8_t)0;
        }
        return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip.netmask.addr));
    }


    uint8_t LAN8720::GetlinkSpeed()
    {
        eth_speed_t link_speed;
        esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
        return (link_speed == ETH_SPEED_10M) ? 10 : 100;
    }


    bool LAN8720::IsFullDuplex()
    {
        eth_duplex_t link_duplex;
        esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, &link_duplex);
        return (link_duplex == ETH_DUPLEX_FULL);
    }

    
    bool LAN8720::IsLinkUp()
    {
        return WiFiGenericClass::getStatusBits() & ETH_CONNECTED_BIT;
    }


    bool LAN8720::begin()
    {
        tcpipInit();

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
        ret = esp_eth_driver_install(&eth_config, &eth_handle);
        if (ret != ESP_OK)
        {
            LOG_ERROR(muffin::logger, "FAILED TO INSTALL ETHERNET DRVIER: %s", 
                esp_err_to_name(ret));
            return false;
        }
        else if (eth_handle == NULL)
        {
            LOG_ERROR(muffin::logger, "FAILED TO INSTALL ETHERNET DRVIER");
            return false;
        }
        

        /* attach Ethernet driver to TCP/IP stack */
        ret = esp_netif_attach(mInterface, esp_eth_new_netif_glue(eth_handle));
        if (ret != ESP_OK)
        {
            LOG_ERROR(muffin::logger, "FAILED TO ATTACH ETHERNET INTERFACE: %s",
                esp_err_to_name(ret));
            return false;
        }

        /* attach to WiFiGeneric to receive events */
        add_esp_interface_netif(ESP_IF_ETH, mInterface);

        char hostName[32] = {'\0'};
    #if defined(MT10)
        snprintf(hostName, 32, "MT10-%s", macAddress.GetEthernet());
    #elif defined(MB10)
        snprintf(hostName, 32, "MB10-%s", macAddress.GetEthernet());
    #endif
        
        ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostName);
        if (ret != ESP_OK)
        {
            LOG_ERROR(muffin::logger, "FAILED TO SET HOSTNAME WITH CODE: %s",
                esp_err_to_name(ret));
            std::abort();
        }
        LOG_INFO(muffin::logger, "Host name: %s", hostName);
        
        ret = esp_eth_start(eth_handle);
        if (ret != ESP_OK)
        {
            LOG_ERROR(muffin::logger, "FAILED TO START ETHERNET INTERFACE: %s",
                esp_err_to_name(ret));
            return false;
        }
        
        // holds a few milliseconds to let DHCP start and enter into a good state
        // FIX ME -- adresses issue https://github.com/espressif/arduino-esp32/issues/5733
        delay(50);
        return true;
    }


    bool LAN8720::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
    {
        esp_err_t ret = ESP_OK;
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

        ret = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
        if (ret != ESP_OK && ret != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED)
        {
            LOG_ERROR(logger, "FAILED TO STOP DHCP CLIENT: %s", 
                esp_err_to_name(ret));
            return false;
        }

        ret = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
        if (ret != ERR_OK)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE IPv4: %s",
                esp_err_to_name(ret));
            return false;
        }

        if (info.ip.addr)
        {
            staticIP = true;
        }
        else
        {
            ret = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
            if (ret != ESP_OK && ret != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED)
            {
                LOG_ERROR(logger, "FAILED TO START DHCP CLIENT: %s",
                    esp_err_to_name(ret));
                return false;
            }
            /**
             * @todo DHCP 클라이언트 상태 확인 필요할 수 있음 <-- e.g. tcpip_adapter_dhcpc_get_status()
             * @brief 동적 IP 할당으로 설정된 디바이스에서 가끔씩 IP 주소를 못 받는 오류가 발생하고 있음.
             *        본 todo 아이템은 혹시 DHCP 클라이언트 상태를 확인해서 재요청을 하면 되지 않을까
             *        하는 마음에 적은 것임
             */
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


    void LAN8720::getArduinoEthernetEvent(arduino_event_id_t event)
    {
        ethernet->implArduinoCallback(event);
    }


    void LAN8720::implArduinoCallback(arduino_event_id_t event)
    {
        switch (event)
        {
        case ARDUINO_EVENT_ETH_START:
            LOG_INFO(logger, "Ethernet PHY has started");
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            return;

        case ARDUINO_EVENT_ETH_STOP:
            LOG_WARNING(logger, "ETHERNET PHY HAS STOPPED");
            mFlags.reset(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlags.reset(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        case ARDUINO_EVENT_ETH_CONNECTED:
            LOG_INFO(logger, "Ethernet is connected");
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            return;

        case ARDUINO_EVENT_ETH_DISCONNECTED:
            LOG_WARNING(logger, "ETHERNET PHY HAS DISCONNECTED");
            mFlags.reset(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlags.reset(static_cast<uint8_t>(flag_e::HAS_IPv4));
            break;

        case ARDUINO_EVENT_ETH_GOT_IP:
            LOG_INFO(logger, "Ethernet got IPv4 address");
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        case ARDUINO_EVENT_ETH_GOT_IP6:
            LOG_INFO(logger, "Ethernet got IPv6 address");
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlags.set(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        default:
            ASSERT((false), "RECEIVED UNEXPECTED EVENT ID: %d", event);
            break;
        }
    }


    LAN8720* ethernet = nullptr;
}


#endif