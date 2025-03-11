/**
 * @file WiFi4.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 통신을 사용하는데 필요한 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <WiFi.h>

#include "WiFi4.h"
#include "Common/Logger/Logger.h"


namespace muffin {

    WiFi4::state_e WiFi4::mState = state_e::NOT_INITIALIZED_YET;

    WiFi4::WiFi4()
    {
        memset(mMacAddress, '\0', sizeof(mMacAddress));
        memset(mHostname,   '\0', sizeof(mHostname));
    
    }

    WiFi4::~WiFi4()
    {
    }

    Status WiFi4::Init()
    {
        uint8_t baseMAC[6] = { 0, 0, 0, 0, 0, 0 };
        const esp_err_t ret = esp_read_mac(baseMAC, ESP_MAC_WIFI_STA);
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

        if (WiFi.mode(mMode) == false)
        {
            LOG_ERROR(logger, "FAILED TO SET Wi-Fi TO %s MODE", mModeString[mMode]);
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        mState = state_e::SUCCEDDED_TO_INITIALIZE;
        return Status(Status::Code::GOOD);
    }

    Status WiFi4::Config(jvs::config::Base* config)
    {
        assert(config != nullptr);
        // assert(config->GetCategory() == cfg_key_e::WIFI4);
        assert(mState != state_e::NOT_INITIALIZED_YET);

        if (mIsArduinoEventCallbackRegistered == false)
        {
            if (WiFi.onEvent(getArduinoWiFiEvent) == 0)
            {
                LOG_ERROR(logger, "FAILED TO REGISTER EVENT CALLBACK");
                mState = state_e::FAILED_TO_CONFIGURE;
                return Status(Status::Code::BAD_INTERNAL_ERROR);
            }
            else
            {
                //LOG_VERBOSE(logger, "Registered Wi-Fi event callback");
                mIsArduinoEventCallbackRegistered = true;
            }
        }
        
        mConfig = *static_cast<jvs::config::WiFi4*>(config);
        WiFi.setMinSecurity(mConfig.GetAuthMode().second);
        WiFi.setAutoReconnect(true);

        bool isConfigured = false;
        if (mConfig.GetDHCP().second == true)
        {
            IPAddress dhcpIPv4(0, 0, 0, 0);
            isConfigured = WiFi.config(
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4,
                dhcpIPv4
            );
        }
        else
        {
            isConfigured = WiFi.config(
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

        mState = state_e::SUCCEDDED_TO_CONFIGURE;
        return Status(Status::Code::GOOD_ENTRY_REPLACED);
    }

    Status WiFi4::Connect()
    {
        assert(mState > state_e::SUCCEDDED_TO_INITIALIZE);

        wl_status_t ret = wl_status_t::WL_IDLE_STATUS;

        if (mConfig.GetAuthMode().second == wifi_auth_mode_t::WIFI_AUTH_OPEN)
        {
            ret = WiFi.begin(mConfig.GetSSID().second.c_str(), NULL);
        }
        else if (mConfig.GetAuthMode().second != wifi_auth_mode_t::WIFI_AUTH_WPA2_ENTERPRISE)
        {
            ret = WiFi.begin(mConfig.GetSSID().second.c_str(), mConfig.GetPSK().second.c_str());
        }
        else
        {
            LOG_ERROR(logger, "NEED TO DEFINE WPA2 AUTHENTICATION METHOD TO USE Wi-Fi EAP");
            delay(UINT32_MAX);
            assert(false);

            ret = WiFi.begin(
                mConfig.GetSSID().second.c_str(), 
                wpa2_auth_method_t::WPA2_AUTH_TLS, 
                mConfig.GetEapID().second.c_str(), 
                mConfig.GetEapUserName().second.c_str(), 
                mConfig.GetEapPassword().second.c_str()
            );
        }

        const uint32_t startMillis = millis();
        while (millis() - startMillis < 1000)
        {
            if (WiFi.status() == wl_status_t::WL_CONNECTED)
            {
                mState = state_e::SUCCEDDED_TO_CONNECT_TO_AP;
                return Status(Status::Code::GOOD);
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        LOG_ERROR(logger, "FAILED TO CONNECT THE AP: %d", ret);
        mState = state_e::FAILED_TO_CONNECT_TO_AP;
        return Status(Status::Code::UNCERTAIN);
    }

    Status WiFi4::Disconnect()
    {
        if (WiFi.disconnect() == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::UNCERTAIN);
        }
    }

    Status WiFi4::Reconnect()
    {
        if (WiFi.reconnect() == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::UNCERTAIN);
        }
    }

    bool WiFi4::IsConnected() const
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    IPAddress WiFi4::GetIPv4() const
    {
        return WiFi.localIP();
    }

    std::pair<Status, size_t> WiFi4::TakeMutex()
    {
        return std::make_pair(Status(Status::Code::GOOD), 1);
    }

    Status WiFi4::ReleaseMutex()
    {
        return Status(Status::Code::GOOD);
    }

    const char* WiFi4::GetMacAddress() const
    {
        return mMacAddress;
    }

    WiFi4::state_e WiFi4::GetState() const
    {
        return mState;
    }

    void WiFi4::getArduinoWiFiEvent(arduino_event_id_t event)
    {
        switch (event)
        {
        case ARDUINO_EVENT_WIFI_READY:
            //LOG_VERBOSE(logger, "Wi-Fi is ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            //LOG_VERBOSE(logger, "Wi-Fi scanning is done");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            LOG_INFO(logger, "Wi-Fi client is started");
            mState = state_e::SUCCEDDED_TO_START_CLIENT;
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            LOG_WARNING(logger, "Wi-Fi CLIENT HAS STOPPED");
            mState = state_e::WIFI_CLIENT_HAS_STOPPED;
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            LOG_INFO(logger, "Wi-Fi client is connected");
            mState = state_e::SUCCEDDED_TO_CONNECT_TO_AP;
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            LOG_WARNING(logger, "Wi-Fi CLIENT HAS DISCONNECTED");
            mState = state_e::WIFI_CLIENT_HAS_DISCONNECTED;
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            LOG_WARNING(logger, "Wi-Fi AUTH MODE HAS CHANGED");
            mState = state_e::WIFI_CLIENT_AUTH_MODE_CHANGED;
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            LOG_INFO(logger, "Wi-Fi got IPv4 address");
            mState = state_e::SUCCEDDED_TO_GET_IP_ADDRESS;
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            LOG_INFO(logger, "Wi-Fi got IPv6 address");
            mState = state_e::SUCCEDDED_TO_GET_IP_ADDRESS;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            LOG_INFO(logger, "Wi-Fi CLIENT HAS LOST IP ADDRESS");
            mState = state_e::WIFI_CLIENT_LOST_IP_ADDRESS;
            break;
        default:
            LOG_ERROR(logger, "RECEIVED UNEXPECTED EVENT ID: %d", event);
            assert(false);
            break;
        }
    }
}