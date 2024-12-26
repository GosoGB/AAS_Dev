/**
 * @file DeviceStatus.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 디바이스의 상태 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-11-30
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <esp_system.h>
#include <string>
#include <sys/_stdint.h>
#include <map>

#include "Common/Status.h"


namespace muffin {

    typedef enum class McuTypeEnum
    {
        MCU_ESP32,
    #if defined(MODLINK_T2) | defined(MODLINK_B)
        MCU_ATmega2560
    #endif
    } mcu_type_e;

	typedef struct FirmwareVersionType
	{
		uint32_t Code;
		std::string Semantic;
	} fw_vsn_t;

#if defined(MODLINK_B) | defined(V_OLA_T10) | defined(V_OLA_H10)
	typedef struct WiFiStatusReportType
	{
      bool Enabled;
		std::string LocalIP;
      std::string Status;
		int16_t RSSI;
	} wifi_report_t;
#endif

#if !defined(V_OLA_T10) | !defined(V_OLA_H10)
   #if defined(MODLINK_T2) | defined(MODLINK_B)
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



	class DeviceStatus
	{
	public:
		DeviceStatus(DeviceStatus const&) = delete;
		void operator=(DeviceStatus const&) = delete;
		static DeviceStatus* CreateInstanceOrNULL();
		static DeviceStatus& GetInstance();
	private:
		DeviceStatus();
		virtual ~DeviceStatus() {}
	private:
		static DeviceStatus* mInstance;


   public:
      fw_vsn_t GetFirmwareVersion(const mcu_type_e type);
      void SetFirmwareVersion(const mcu_type_e type, const char* semver, const uint32_t vc);
   public:
      void SetResetReason(const esp_reset_reason_t code);
      void SetStatusCode(const Status::Code statusCode);
      void SetReconfigurationCode(const uint8_t reconfigurationCode);
      void SetTask(const std::string name, size_t remainedStack);
      void SetRemainedHeap(const size_t memory);
      void SetRemainedFlash(const size_t memory);
#if !defined(V_OLA_T10) | !defined(V_OLA_H10)
   #if defined(MODLINK_T2) | defined(MODLINK_B)
      void SetReportEthernet(const eth_report_t report);
   #endif
      void SetReportCatM1(const catm1_report_t report);
#endif
   #if defined(MODLINK_B) | defined(V_OLA_T10) | defined(V_OLA_H10)
      void SetReportWiFi(const wifi_report_t report);
   #endif


   public:
      std::string ToString();

	private:
		fw_vsn_t mVersionESP32;
	#if defined(MODLINK_T2)
		fw_vsn_t mVersionMEGA2560;
	#endif
      esp_reset_reason_t mResetReason;
      uint8_t mReconfigurationCode = 0;
      Status mStatus;
      std::map<std::string, size_t> mMapTaskResources;
      size_t mRemainedHeapMemory = 0;
      size_t mRemainedFlashMemory = 0;

#if !defined(V_OLA_T10) | !defined(V_OLA_H10)
   #if defined(MODLINK_T2) | defined(MODLINK_B)
      eth_report_t mEthernetStatusReport;
   #endif
      catm1_report_t mCatM1StatusReport;
#endif

   #if defined(MODLINK_B) | defined(V_OLA_T10) | defined(V_OLA_H10)
      wifi_report_t mWiFiStatusReport;
   #endif
	};
}