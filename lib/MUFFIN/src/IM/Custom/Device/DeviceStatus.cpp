/**
 * @file DeviceStatus.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 디바이스의 상태 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-01-15
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <ArduinoJson.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Core/Include/Helper.h"
#include "DeviceStatus.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"




namespace muffin {

    DeviceStatus::DeviceStatus()
        : mStatus(Status::Code::UNCERTAIN)
    {
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

        strcpy(mTaskResources[0].TaskName, "MqttTask");
        mTaskResources[0].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[0].RemainedStackSize = -1;

        strcpy(mTaskResources[1].TaskName, "cyclicalsMSGTask");
        mTaskResources[1].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[1].RemainedStackSize = -1;

        strcpy(mTaskResources[2].TaskName, "ModbusRtuTask");
        mTaskResources[2].TotalStackSize    = 5 * KILLOBYTE;
        mTaskResources[2].RemainedStackSize = -1;

        strcpy(mTaskResources[3].TaskName, "ModbusTcpTask");
        mTaskResources[3].TotalStackSize    = 5 * KILLOBYTE;
        mTaskResources[3].RemainedStackSize = -1;

        strcpy(mTaskResources[4].TaskName, "MornitorAlarmTask");
        mTaskResources[4].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[4].RemainedStackSize = -1;

        strcpy(mTaskResources[5].TaskName, "OpTimeTask");
        mTaskResources[5].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[5].RemainedStackSize = -1;

        strcpy(mTaskResources[6].TaskName, "ProductionInfoTask");
        mTaskResources[6].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[6].RemainedStackSize = -1;

        strcpy(mTaskResources[7].TaskName, "CatM1ProcessorTask");
        mTaskResources[7].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[7].RemainedStackSize = -1;

        strcpy(mTaskResources[8].TaskName, "CatM1ConnectedTask");
        mTaskResources[8].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[8].RemainedStackSize = -1;

        strcpy(mTaskResources[9].TaskName, "MelsecTask");
        mTaskResources[9].TotalStackSize    = 4 * KILLOBYTE;
        mTaskResources[9].RemainedStackSize = -1;

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
    
    void DeviceStatus::SetReconfigurationCode(const reconfiguration_code_e reconfigurationCode)
    {
        mReconfigurationCode = reconfigurationCode;
    }

    void DeviceStatus::SetTaskRemainedStack(task_name_e task, size_t remainedStack)
    {
        switch (task)
        {
        case task_name_e::MQTT_TASK:
            mTaskResources[0].RemainedStackSize = remainedStack;
            break;
        case task_name_e::CYCLICALS_MSG_TASK:
            mTaskResources[1].RemainedStackSize = remainedStack;
            break;
        case task_name_e::MODBUS_RTU_TASK:
            mTaskResources[2].RemainedStackSize = remainedStack;
            break;
        case task_name_e::MODBUS_TCP_TASK:
            mTaskResources[3].RemainedStackSize = remainedStack;
            break;
        case task_name_e::MORNITOR_ALARM_TASK:
            mTaskResources[4].RemainedStackSize = remainedStack;
            break;
        case task_name_e::OPERATION_TIME_TASK:
            mTaskResources[5].RemainedStackSize = remainedStack;
            break;
        case task_name_e::PRODUCTION_INFO_TASK:
            mTaskResources[6].RemainedStackSize = remainedStack;
            break;
        case task_name_e::CATM1_PROCESSOR_TASK:
            mTaskResources[7].RemainedStackSize = remainedStack;
            break;
        case task_name_e::CATM1_MONITORING_TASK:
            mTaskResources[8].RemainedStackSize = remainedStack;
            break;
            case task_name_e::MELSEC_TASK:
            mTaskResources[9].RemainedStackSize = remainedStack;
            break;
        default:
            LOG_ERROR(logger,"NOT DEFINED TAKSNAME : %d",task);
            break;
        }
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

    std::string DeviceStatus::ToStringEvent()
    {
        JsonDocument doc;
        doc["mac"] = macAddress.GetEthernet();
        doc["ts"]  = GetTimestampInMillis();

        JsonObject firmware               = doc["firmware"].to<JsonObject>();
        JsonObject firmwareESP32          = firmware["esp32"].to<JsonObject>();
        firmwareESP32["semanticVersion"]  = FW_VERSION_ESP32.GetSemanticVersion();
        firmwareESP32["versionCode"]      = FW_VERSION_ESP32.GetVersionCode();
	#if defined(MODLINK_T2)
        JsonObject firmwareATmega2560          = firmware["atmega2560"].to<JsonObject>();
        firmwareATmega2560["semanticVersion"]  = FW_VERSION_MEGA2560.GetSemanticVersion();
        firmwareATmega2560["versionCode"]      = FW_VERSION_MEGA2560.GetVersionCode();
	#endif

        JsonObject event = doc["event"].to<JsonObject>();
        event["deviceReset"]   =  mResetReason;
        event["statusCode"]    =  mStatus.c_str();
        event["reconfigured"]  =  static_cast<uint8_t>(mReconfigurationCode);

        std::string payload;
        serializeJson(doc, payload);

        return payload;
    }

    std::string DeviceStatus::ToStringCyclical()
    {
        JsonDocument doc;
        doc["mac"] = macAddress.GetEthernet();
        doc["ts"]  = GetTimestampInMillis();

        JsonObject firmware               = doc["firmware"].to<JsonObject>();
        JsonObject firmwareESP32          = firmware["esp32"].to<JsonObject>();
        firmwareESP32["semanticVersion"]  = FW_VERSION_ESP32.GetSemanticVersion();
        firmwareESP32["versionCode"]      = FW_VERSION_ESP32.GetVersionCode();
	#if defined(MODLINK_T2)
        JsonObject firmwareATmega2560          = firmware["atmega2560"].to<JsonObject>();
        firmwareATmega2560["semanticVersion"]  = FW_VERSION_MEGA2560.GetSemanticVersion();
        firmwareATmega2560["versionCode"]      = FW_VERSION_MEGA2560.GetVersionCode();
	#endif

        JsonObject cyclical  = doc["cyclical"].to<JsonObject>();
        JsonObject resources = cyclical["resources"].to<JsonObject>();
        JsonArray tasks      = resources["tasks"].to<JsonArray>();

        for (auto& array : mTaskResources)
        {
            if (array.RemainedStackSize != -1)
            {
                JsonObject obj = tasks.add<JsonObject>();
                obj["name"] = array.TaskName;
                obj["totalStackSize"] = array.TotalStackSize;
                obj["RemainedstackSize"] = array.RemainedStackSize;
            }
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
                /**
                 * @todo IPv6 추후에 개발 예정
                 * 
                 */
                // catM1["ip"]    = mCatM1StatusReport.PublicIP;
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