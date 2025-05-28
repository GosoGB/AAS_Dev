/**
 * @file Ethernet.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#if defined(MT10) || defined(MB10)

#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/SPEAR/SPEAR.h"

static uint32_t sNonassignedIPv4Millis = 0;
static TimerHandle_t xTimer = NULL;



namespace muffin {

    void vMonitorNonassignedIPv4(TimerHandle_t xTimer)
    {
        configASSERT(xTimer);
        
        if (ethernet->IsConnected() == false)
        {
            if ((millis() - sNonassignedIPv4Millis) > 10*SECOND_IN_MILLIS)
            {
                esp_eth_stop(deprecableEthernet.eth_handle);
                esp_eth_start(deprecableEthernet.eth_handle);
                sNonassignedIPv4Millis = millis();
            }
        }
        else
        {
            xTimerStop(xTimer, 0);
        }
    }

    Ethernet::Ethernet()
    {
        mFlogs.reset();
    }

    Status Ethernet::Init()
    {
        if (mFlogs.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true)
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
        mFlogs.set(static_cast<uint8_t>(flag_e::INITIALIZED));
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Config(jvs::config::Base* config)
    {
        ASSERT((config != nullptr), "INPUT PARAM <jvs::config::Base* config> CANNOT BE NULL");
        ASSERT((config->GetCategory() == jvs::cfg_key_e::ETHERNET), "INVALID JARVIS CATEGORY");
        ASSERT((mFlogs.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true), "MUST BE INITIALIZED FIRST");
        mFlogs.set(static_cast<uint8_t>(flag_e::CONFIGURED));
        return Status(Status::Code::GOOD);
    }

    Status Ethernet::Connect()
    {
        ASSERT((mFlogs.test(static_cast<uint8_t>(flag_e::INITIALIZED)) == true), "MUST BE INITIALIZED FIRST");
        ASSERT((mFlogs.test(static_cast<uint8_t>(flag_e::CONFIGURED))  == true), "MUST BE CONFIGURED FIRST");

        if (mFlogs.test(static_cast<uint8_t>(flag_e::HAS_STARTED)) == true)
        {
            return Status(Status::Code::GOOD);
        }
        
        if (deprecableEthernet.Begin() == false)
        {
            goto ON_FAIL;
        }

        if (jvs::config::ethernet->GetDHCP().second == false)
        {
            const bool isConfigured = deprecableEthernet.config(jvs::config::ethernet->GetStaticIPv4().second,
                                                                jvs::config::ethernet->GetGateway().second,
                                                                jvs::config::ethernet->GetSubnetmask().second,
                                                                jvs::config::ethernet->GetDNS1().second,
                                                                jvs::config::ethernet->GetDNS2().second);

            if (isConfigured == false)
            {
                goto ON_FAIL;
            }
        }

        if (xTimer == NULL)
        {
            xTimer = xTimerCreate(
                "nonassigned_ipv4",      // pcTimerName
                SECOND_IN_MILLIS,        // xTimerPeriod,
                pdTRUE,                  // uxAutoReload,
                (void *)0,               // pvTimerID,
                vMonitorNonassignedIPv4  // pxCallbackFunction
            );
        }
        xTimerStart(xTimer, 0);

        mFlogs.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
        return Status(Status::Code::GOOD);

    ON_FAIL:
        LOG_ERROR(logger, "FAILED TO START ETHERNET PHY");
        return Status(Status::Code::BAD_DEVICE_FAILURE);
    }

    Status Ethernet::Disconnect()
    {
        LOG_ERROR(logger, "DISCONNECT IS NOT SUPPORTED SERVICE");
    #if defined(DEBUG)
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    #else
        return Status(Status::Code::GOOD);
    #endif
    }

    Status Ethernet::Reconnect()
    {
        LOG_ERROR(logger, "RECONNECT IS NOT SUPPORTED SERVICE");
    #if defined(DEBUG)
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    #else
        return Status(Status::Code::GOOD);
    #endif
    }

    bool Ethernet::IsConnected() const
    {
        if (mFlogs.test(static_cast<uint8_t>(flag_e::HAS_IPv4)) == true)
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
        return deprecableEthernet.GetIPv4();
    }

    Status Ethernet::SyncNTP()
    {
        const char* ntpServer = "time.google.com";
        const char* ntpServer2 = "time.windows.com";
        const long gmtOffset_sec = 32400;
        const int daylightOffset_sec = 0;

        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2);
        
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

    std::pair<Status, size_t> Ethernet::TakeMutex()
    {
        return std::make_pair(Status(Status::Code::GOOD), 1);
    }

    Status Ethernet::ReleaseMutex()
    {
        return Status(Status::Code::GOOD);
    }

    void Ethernet::getArduinoEthernetEvent(arduino_event_id_t event)
    {
        ethernet->implArduinoCallback(event);
    }

    void Ethernet::implArduinoCallback(arduino_event_id_t event)
    {
        switch (event)
        {
        case ARDUINO_EVENT_ETH_START:
            LOG_INFO(logger, "Ethernet PHY has started");
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            sNonassignedIPv4Millis = millis();
            return;

        case ARDUINO_EVENT_ETH_STOP:
            LOG_WARNING(logger, "ETHERNET PHY HAS STOPPED");
            mFlogs.reset(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlogs.reset(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        case ARDUINO_EVENT_ETH_CONNECTED:
            LOG_INFO(logger, "Ethernet is connected");
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            sNonassignedIPv4Millis = millis();
            return;

        case ARDUINO_EVENT_ETH_DISCONNECTED:
            LOG_WARNING(logger, "ETHERNET PHY HAS DISCONNECTED");
            mFlogs.reset(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlogs.reset(static_cast<uint8_t>(flag_e::HAS_IPv4));
            break;

        case ARDUINO_EVENT_ETH_GOT_IP:
            LOG_INFO(logger, "Ethernet got IPv4 address");
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        case ARDUINO_EVENT_ETH_GOT_IP6:
            LOG_INFO(logger, "Ethernet got IPv6 address");
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_STARTED));
            mFlogs.set(static_cast<uint8_t>(flag_e::HAS_IPv4));
            return;

        default:
            ASSERT((false), "RECEIVED UNEXPECTED EVENT ID: %d", event);
            break;
        }
    }


    Ethernet* ethernet = nullptr;
}

#endif