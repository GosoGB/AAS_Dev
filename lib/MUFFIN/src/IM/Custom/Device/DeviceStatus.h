/**
 * @file DeviceStatus.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 디바이스의 상태 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2025-01-15
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <esp_system.h>
#include <string>
#include <sys/_stdint.h>
#include <map>

#include "Common/Status.h"
#include "IM/Custom/Constants.h"


namespace muffin {

#if defined(MB10) || defined(V_OLA_T10) || defined(V_OLA_H10)
	typedef struct WiFiStatusReportType
	{
      bool Enabled;
		std::string LocalIP;
      std::string Status;
		int16_t RSSI;
	} wifi_report_t;
#endif

#if !defined(V_OLA_T10) || !defined(V_OLA_H10)
   #if defined(MT10) || defined(MB10) || defined(MT11)
      typedef struct EthernetStatusReportType
      {
         bool Enabled;
         std::string LocalIP;
         std::string Status;
      } eth_report_t;
   #endif

      typedef struct CatM1StatusReportType
      {
         bool Enabled;
         std::string PublicIP;
         int16_t RSSI;
         int16_t RSRP;
         int16_t SINR;
         int16_t RSRQ;
      } catm1_report_t;
#endif

   typedef struct TaskInfoType
   {
      char TaskName[20];
      size_t TotalStackSize;
      int32_t RemainedStackSize;
   } task_info_t;

	class DeviceStatus
	{
	public:
		DeviceStatus();
		virtual ~DeviceStatus() {}
   public:
      void SetTaskRemainedStack(task_name_e task, size_t remainedStack);
      void SetResetReason(const esp_reset_reason_t code);
      void SetStatusCode(const Status::Code statusCode);
      void SetReconfigurationCode(const reconfiguration_code_e reconfigurationCode);
      void SetRemainedHeap(const size_t memory);
      void SetRemainedFlash(const size_t memory);
#if !defined(V_OLA_T10) || !defined(V_OLA_H10)
   #if defined(MT10) || defined(MB10) || defined(MT11)
      void SetReportEthernet(const eth_report_t report);
   #endif
      void SetReportCatM1(const catm1_report_t report);
#endif
   #if defined(MB10) || defined(V_OLA_T10) || defined(V_OLA_H10)
      void SetReportWiFi(const wifi_report_t report);
   #endif


   public:
      std::string ToStringEvent();
      std::string ToStringCyclical();
	private:
      esp_reset_reason_t mResetReason;
      reconfiguration_code_e mReconfigurationCode;
      Status mStatus;
      size_t mRemainedHeapMemory = 0;
      size_t mRemainedFlashMemory = 0;
      task_info_t mTaskResources[11];

#if !defined(V_OLA_T10) || !defined(V_OLA_H10)
   #if defined(MT10) || defined(MB10) || defined(MT11)
      eth_report_t mEthernetStatusReport;
   #endif
      catm1_report_t mCatM1StatusReport;
#endif

   #if defined(MB10) || defined(V_OLA_T10) || defined(V_OLA_H10)
      wifi_report_t mWiFiStatusReport;
   #endif
	};


   extern DeviceStatus deviceStatus;
}