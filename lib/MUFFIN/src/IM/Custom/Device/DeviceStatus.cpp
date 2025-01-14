/**
 * @file DeviceStatus.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 디바이스의 상태 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-01-15
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <ArduinoJson.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Core/Include/Helper.h"
#include "DeviceStatus.h"
#include "IM/Custom/MacAddress/MacAddress.h"



namespace muffin {

    DeviceStatus::DeviceStatus()
        : mStatus(Status::Code::UNCERTAIN)
    {
        mVersionESP32.Semantic = "1.2.1";
        mVersionESP32.Code = 121;
#if defined(MODLINK_T2) || defined(MODLINK_B)
        mVersionMEGA2560.Semantic = "0.0.0";
        mVersionMEGA2560.Code = 999;
#endif

#if !defined(V_OLA_T10) || !defined(V_OLA_H10)
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        mEthernetStatusReport.Enabled = false;
        mEthernetStatusReport.LocalIP = "0.0.0.0";
        mEthernetStatusReport.Status  = "UNKNOWN";
    #endif

        mCatM1StatusReport.Enabled   = false;
        mCatM1StatusReport.PublicIP  = "0.0.0.0";
        mCatM1StatusReport.RSRP      = INT16_MIN;
        mCatM1StatusReport.RSRQ      = INT16_MIN;
        mCatM1StatusReport.RSSI      = INT16_MIN;
        mCatM1StatusReport.SINR      = INT16_MIN;
#endif

   #if defined(MODLINK_B) || defined(V_OLA_T10) || defined(V_OLA_H10)
      mWiFiStatusReport.Enabled  = false;
      mWiFiStatusReport.LocalIP  = "0.0.0.0";
      mWiFiStatusReport.Status   = "UNKNOWN";
      mWiFiStatusReport.RSSI     = INT16_MIN;
   #endif
    }

    void DeviceStatus::SetResetReason(const esp_reset_reason_t code)
    {
        mResetReason = code;

        switch (mResetReason)
        {
        case ESP_RST_UNKNOWN:
            LOG_INFO(muffin::logger, "Reset reason: Cannot be determined");
            return;
        case ESP_RST_POWERON:
            LOG_INFO(muffin::logger, "Reset reason: Due to power-on event");
            return;
        case ESP_RST_EXT:
            LOG_INFO(muffin::logger, "Reset reason: By External pin");
            return;
        case ESP_RST_SW:
            LOG_INFO(muffin::logger, "Reset reason: By software reset");
            return;
        case ESP_RST_PANIC:
            LOG_INFO(muffin::logger, "Reset reason: Due to exception or panic");
            return;
        case ESP_RST_INT_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to interrupt watchdog");
            return;
        case ESP_RST_TASK_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to task watchdog");
            return;
        case ESP_RST_WDT:
            LOG_INFO(muffin::logger, "Reset reason: Due to other watchdog");
            return;
        case ESP_RST_DEEPSLEEP:
            LOG_INFO(muffin::logger, "Reset reason: By deep sleep mode");
            return;
        case ESP_RST_BROWNOUT:
            LOG_INFO(muffin::logger, "Reset reason: By Brownout");
            return;
        case ESP_RST_SDIO:
            LOG_INFO(muffin::logger, "Reset reason: Due to error with SDIO interface");
            return;
        default:
            ASSERT(false, "UNDEFINED RESET REASON CODE: %u", mResetReason);
            return;
        }
    }

    void DeviceStatus::SetStatusCode(const Status::Code statusCode)
    {
        mStatus = statusCode;
    }
    
    void DeviceStatus::SetReconfigurationCode(const uint8_t reconfigurationCode)
    {
        mReconfigurationCode = reconfigurationCode;
    }
    
    void DeviceStatus::SetTask(const std::string name, size_t remainedStack)
    {
        mMapTaskResources[name] = remainedStack;
    }
    
    void DeviceStatus::SetRemainedHeap(const size_t memory)
    {
        mRemainedHeapMemory = memory;
    }

    void DeviceStatus::SetRemainedFlash(const size_t memory)
    {
        mRemainedFlashMemory = memory;
    }

#if !defined(V_OLA_T10) || !defined(V_OLA_H10)
    #if defined(MODLINK_T2) || defined(MODLINK_B)
    void DeviceStatus::SetReportEthernet(const eth_report_t report)
    {
        mEthernetStatusReport = report;
    }
   #endif

    void DeviceStatus::SetReportCatM1(const catm1_report_t report)
    {
        mCatM1StatusReport = report;
    }
#endif

#if defined(MODLINK_B) || defined(V_OLA_T10) || defined(V_OLA_H10)
    void DeviceStatus::SetReportWiFi(const wifi_report_t report)
    {
        mWiFiStatusReport = report;
    }
#endif

    std::string DeviceStatus::ToString()
    {
        JsonDocument doc;
        doc["mac"] = macAddress.GetEthernet();
        doc["ts"]  = GetTimestampInMillis();

        JsonObject firmware               = doc["firmware"].to<JsonObject>();
        JsonObject firmwareESP32          = firmware["esp32"].to<JsonObject>();
        firmwareESP32["semanticVersion"]  = mVersionESP32.Semantic;
        firmwareESP32["versionCode"]      = mVersionESP32.Code;
	#if defined(MODLINK_T2)
        JsonObject firmwareATmega2560          = firmware["atmega2560"].to<JsonObject>();
        firmwareATmega2560["semanticVersion"]  = mVersionMEGA2560.Semantic;
        firmwareATmega2560["versionCode"]      = mVersionMEGA2560.Code;
	#endif

        JsonObject event = doc["event"].to<JsonObject>();
        event["deviceReset"]   =  mResetReason;
        event["statusCode"]    =  mStatus.c_str();
        event["reconfigured"]  =  mReconfigurationCode;

        JsonObject cyclical  = doc["cyclical"].to<JsonObject>();
        JsonObject resources = cyclical["resources"].to<JsonObject>();
        JsonArray tasks      = resources["tasks"].to<JsonArray>();
        for (const auto& pair : mMapTaskResources)
        {
            JsonObject obj = tasks.add<JsonObject>();
            obj["name"] = pair.first;
            obj["stackRemained"] = pair.second;
        }
        resources["heapRemained"]   = mRemainedHeapMemory;
        resources["flashRemained"]  = mRemainedFlashMemory;

        JsonObject network  = doc["network"].to<JsonObject>();
        {
    #if !defined(V_OLA_T10) || !defined(V_OLA_H10)
        #if defined(MODLINK_T2) || defined(MODLINK_B)
            if (mEthernetStatusReport.Enabled == true)
            {
                JsonObject ethernet = network["ethernet"].to<JsonObject>();
                ethernet["ip"]      = mEthernetStatusReport.LocalIP;
                ethernet["status"]  = mEthernetStatusReport.Status;
            }
        #endif
            if (mCatM1StatusReport.Enabled == true)
            {
                JsonObject catM1 = network["catM1"].to<JsonObject>();
                catM1["ip"]    = mCatM1StatusReport.PublicIP;
                catM1["rssi"]  = mCatM1StatusReport.RSSI;
                catM1["rsrp"]  = mCatM1StatusReport.RSRP;
                catM1["sinr"]  = mCatM1StatusReport.SINR;
                catM1["rsrq"]  = mCatM1StatusReport.RSRQ;
            }
    #endif

        #if defined(MODLINK_B) || defined(V_OLA_T10) || defined(V_OLA_H10)
            if (mWiFiStatusReport.Enabled == true)
            {
                JsonObject wifi = network["wifi"].to<JsonObject>();
                wifi["ip"]      = mWiFiStatusReport.LocalIP;
                wifi["status"]  = mWiFiStatusReport.Status;
                wifi["rssi"]    = mWiFiStatusReport.RSSI;
            }
        #endif
        }

        std::string payload;
        serializeJson(doc, payload);
        return payload;
    }


    DeviceStatus deviceStatus;
}