/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */



#include "Protocol/SPEAR/SPEAR.h"
#include "Ethernet.h"
#include "Common/Time/TimeUtils.h"


namespace muffin {

    Ethernet::state_e Ethernet::mState = state_e::NOT_INITIALIZED_YET;

    Ethernet::Ethernet()
    {
        memset(mMacAddress, '\0', sizeof(mMacAddress));
        memset(mHostname,   '\0', sizeof(mHostname));
    }
    
    Ethernet::~Ethernet()
    {
    }

    Status Ethernet::Init()
    {
        uint8_t baseMAC[6] = { 0, 0, 0, 0, 0, 0 };
        const esp_err_t ret = esp_read_mac(baseMAC, ESP_MAC_ETH);
        if (ret != ESP_OK)
        {
            LOG_ERROR(logger, "FAILED TO READ MAC ADDRESS: %s", esp_err_to_name(ret));
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        sprintf(mMacAddress, "%02X%02X%02X%02X%02X%02X", 
            baseMAC[0], baseMAC[1], baseMAC[2], baseMAC[3], baseMAC[4], baseMAC[5]
        );
        
        sprintf(mHostname, "%02X-%02X-%02X-%02X-%02X-%02X", 
            baseMAC[0], baseMAC[1], baseMAC[2], baseMAC[3], baseMAC[4], baseMAC[5]
        );

        mState = state_e::SUCCEDDED_TO_INITIALIZE;
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Config(jvs::config::Base* config)
    {
        assert(config != nullptr);
        assert(config->GetCategory() == jvs::cfg_key_e::ETHERNET);
        assert(mState != state_e::NOT_INITIALIZED_YET);

        if (mIsArduinoEventCallbackRegistered == false)
        {
            if (WiFi.onEvent(getArduinoEthernetEvent) == 0)
            {
                LOG_ERROR(logger, "FAILED TO REGISTER EVENT CALLBACK");
                mState = state_e::FAILED_TO_CONFIGURE;
                return Status(Status::Code::BAD_INTERNAL_ERROR);
            }
            else
            {
                //LOG_VERBOSE(logger, "Registered Ethernet event callback");
                mIsArduinoEventCallbackRegistered = true;
            }
        }

        mConfig = *static_cast<jvs::config::Ethernet*>(config);
        mState = state_e::SUCCEDDED_TO_CONFIGURE;
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Connect()
    {
        assert(mState > state_e::SUCCEDDED_TO_INITIALIZE);
        LOG_WARNING(logger, "mHasBegun : %s", mHasBegun == true ? "true": "false");
        if (mHasBegun == false)
        {
            if (ETH.begin(mPhyAddress, mPhyPower, mPhyMDC, mPhyMDIO, mPhyChipsetType, mPhyClockMode) == false)
            {
                mState = state_e::FAILED_TO_START_PHY;
                LOG_ERROR(logger, "FAILED TO START ETHERNET PHY");
                return Status(Status::Code::UNCERTAIN);
            }
            mState = state_e::SUCCEDDED_TO_START_PHY;
            mHasBegun = true;
        }

        bool isConfigured = false;
        if (mConfig.GetDHCP().second == true)
        {
            IPAddress dhcpIPv4(0, 0, 0, 0);
            isConfigured = ETH.config(
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4
            );
        }
        else
        {
            isConfigured = ETH.config(
                mConfig.GetStaticIPv4().second,
                mConfig.GetGateway().second,
                mConfig.GetSubnetmask().second,
                mConfig.GetDNS1().second,
                mConfig.GetDNS2().second
            );
        }

        if (isConfigured == false)
        {
            mState = state_e::FAILED_TO_CONFIGURE;
            LOG_ERROR(logger, "FAILED TO CONFIGURE");
            return Status(Status::Code::BAD_INTERNAL_ERROR);
        }
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Disconnect()
    {
        LOG_ERROR(logger, "RECONNECT IS NOT SUPPORTED SERVICE");
        assert(false);
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status Ethernet::Reconnect()
    {
        LOG_ERROR(logger, "RECONNECT IS NOT SUPPORTED SERVICE");
        assert(false);
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    bool Ethernet::IsConnected() const
    {
        if (mState == state_e::SUCCEDDED_TO_GET_IP_ADDRESS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::pair<Status, size_t> Ethernet::TakeMutex()
    {
        return std::make_pair(Status(Status::Code::GOOD), 1);
    }

    Status Ethernet::ReleaseMutex()
    {
        return Status(Status::Code::GOOD);
    }

    IPAddress Ethernet::GetIPv4() const
    {
        return ETH.localIP();
    }

    jvs::config::Base* Ethernet::GetConfig()
    {
        return static_cast<jvs::config::Base*>(&mConfig);
    }

    const char* Ethernet::GetMacAddress() const
    {
        return mMacAddress;
    }

    Ethernet::state_e Ethernet::GetState() const
    {
        return mState;
    }

    void Ethernet::getArduinoEthernetEvent(arduino_event_id_t event)
    {
        switch (event)
        {
        case ARDUINO_EVENT_ETH_START:
            LOG_INFO(logger, "Ethernet PHY has started");
            mState = state_e::SUCCEDDED_TO_START_PHY;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            LOG_WARNING(logger, "ETHERNET PHY HAS STOPPED");
            mState = state_e::ETHERNET_PHY_HAS_STOPPED;
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            LOG_INFO(logger, "Ethernet is connected");
            mState = state_e::SUCCEDDED_TO_CONNECT;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            LOG_WARNING(logger, "ETHERNET PHY HAS DISCONNECTED");
            mState = state_e::ETHERNET_PHY_DISCONNECTED;
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            LOG_INFO(logger, "Ethernet got IPv4 address");
            mState = state_e::SUCCEDDED_TO_GET_IP_ADDRESS;
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            LOG_INFO(logger, "Ethernet got IPv6 address");
            mState = state_e::SUCCEDDED_TO_GET_IP_ADDRESS;
            break;
        default:
            LOG_ERROR(logger, "RECEIVED UNEXPECTED EVENT ID: %d", event);
            assert(false);
            break;
        }
    }

    Status Ethernet::SyncWithNTP()
    {
        const char* ntpServer = "time.windows.com";
        const char* ntpServer2 = "time.windows.com";
        const long  gmtOffset_sec = 32400;
        const int daylightOffset_sec = 0;

        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2);

        uint16_t count =0;
        while ( std::to_string(GetTimestamp()).length() != 10 )
        {	
            /* wait for ntp synchronization */
            LOG_WARNING(logger,"Not Synced with NTP Server!!! Current Timestamp : %u",GetTimestamp());
            count++;
            LOG_WARNING(logger,"count : %d",count);
            if(count>15)
            {
#if defined(MODLINK_T2)
                spear.Reset();   
#endif
                ESP.restart();
            }
            delay(1000);
        }
        LOG_INFO(logger, "Synced with NTP Server!!! Current Timestamp : %u",GetTimestamp());

        return Status(Status::Code::GOOD);
    }

    Ethernet* ethernet = nullptr;
}