/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-09-04
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Ethernet.h"



namespace muffin {

    Ethernet::state_e Ethernet::mState = state_e::NOT_INITIALIZED_YET;

    Ethernet::Ethernet()
    {
        memset(mMacAddress, '\0', sizeof(mMacAddress));
        memset(mHostname,   '\0', sizeof(mHostname));
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Ethernet::~Ethernet()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status Ethernet::Init()
    {
        LOG_DEBUG(logger, "Initializing Ethernet interface");

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
        LOG_DEBUG(logger, "MAC Address: %s", mMacAddress);
        
        sprintf(mHostname, "%02X-%02X-%02X-%02X-%02X-%02X", 
            baseMAC[0], baseMAC[1], baseMAC[2], baseMAC[3], baseMAC[4], baseMAC[5]
        );
        LOG_DEBUG(logger, "Hostname: %s", mHostname);

        mState = state_e::SUCCEDDED_TO_INITIALIZE;
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Config(jarvis::config::Base* config)
    {
        assert(config != nullptr);
        assert(config->GetCategory() == jarvis::cfg_key_e::ETHERNET);
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
                LOG_VERBOSE(logger, "Registered Ethernet event callback");
                mIsArduinoEventCallbackRegistered = true;
            }
        }

        mConfig = *static_cast<jarvis::config::Ethernet*>(config);
        mState = state_e::SUCCEDDED_TO_CONFIGURE;
        return Status(Status::Code::GOOD_ENTRY_REPLACED);
    }

    Status Ethernet::Connect()
    {
        assert(mState > state_e::SUCCEDDED_TO_INITIALIZE);

        if (ETH.begin(mPhyAddress, mPhyPower, mPhyMDC, mPhyMDIO, mPhyChipsetType, mPhyClockMode) == false)
        {
            mState = state_e::FAILED_TO_START_PHY;
            LOG_ERROR(logger, "FAILED TO START ETHERNET PHY");
            return Status(Status::Code::UNCERTAIN);
        }
        mState = state_e::SUCCEDDED_TO_START_PHY;

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
        LOG_ERROR(logger, "DISCONNECT IS NOT SUPPORTED SERVICE");
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

    IPAddress Ethernet::GetIPv4() const
    {
        return ETH.localIP();
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
}