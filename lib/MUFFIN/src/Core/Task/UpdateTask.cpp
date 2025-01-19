/**
 * @file UpdateTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-12-27
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include <Preferences.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <esp32-hal.h>

#include "DataFormat/JSON/JSON.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/CRC32/CRC32.h"
#include "Core/Core.h"
#include "IM/Custom/Device/DeviceStatus.h"
#include "CyclicalPubTask.h"

#include "ModbusTask.h"
#include "OTA/MEGA2560/HexParser.h"
#include "OTA/MEGA2560/MEGA2560.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Storage/CatFS/CatFS.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "Protocol/SPEAR/SPEAR.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"


namespace muffin {

    fota_Info_t info;

    TaskHandle_t xTaskFotaHandle = NULL;

    bool updateESP32()
    {
        CatM1& catM1 = CatM1::GetInstance();
        catM1.KillUrcTask(true);
        
        CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
        Status ret = catFS->Begin();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger," FAIL TO BEGIN CATFS");
            return false;
        }
        
        ret = catFS->Open("esp32", true);
        if (ret != Status::Code::GOOD)
        {
            return false;
        }

        ret = catFS->Close();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO CLOSE FILE FROM CATFS");
            return false;
        }
        return true;
    }

#if defined(MODLINK_T2) || defined(MODLINK_B)
    bool updateATmega2560()
    {
        CatM1& catM1 = CatM1::GetInstance();
        catM1.KillUrcTask(true);

        CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
        Status ret = catFS->Open("mega2560", true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO OPEN FILE FROM CATFS");
            return false;
        }

        ota::HexParser hexParser;
        size_t bytesRead = 0;
        size_t maxBufferSize = 4 * 1024;
        while (bytesRead < info.mcu2.FileTotalSize)
        {
            const size_t bytesRemained = info.mcu2.FileTotalSize - bytesRead;
            const size_t bufferSize = bytesRemained > maxBufferSize ?  
                maxBufferSize :
                bytesRemained;
            
            uint8_t buffer[bufferSize] = { 0 };
            Status ret = catFS->Read(bufferSize, buffer);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", ret.c_str());
                return false;
            }
            bytesRead += bufferSize;

            std::string chunk;
            for (size_t i = 0; i < bufferSize; ++i)
            {
                chunk += buffer[i];
            }
            ret = hexParser.Parse(chunk);
            if (ret != Status::Code::GOOD && ret != Status::Code::GOOD_MORE_DATA)
            {
                LOG_ERROR(logger, "FAILED TO PARSE HEX RECORDS FOR ATmega2560: %s", ret.c_str());
                return false;
            }
            LOG_DEBUG(logger, "HEX Parser: %s", ret.c_str());
        }

        ota::MEGA2560 mega2560;
        size_t currentAddress = 0;
        ret = mega2560.Init(info.mcu2.FileTotalSize);

        while (hexParser.GetPageCount())
        {
            ota::page_t page = hexParser.GetPage();
            LOG_DEBUG(muffin::logger, "Page Size: %u", page.Size);

            mega2560.LoadAddress(currentAddress);
            mega2560.ProgramFlashISP(page);

            ota::page_t pageReadBack;
            for (size_t i = 0; i < 5; i++)
            {
                Status ret = mega2560.LoadAddress(currentAddress);
                if (ret != Status::Code::GOOD)
                {
                    continue;
                }
                
                ret = mega2560.ReadFlashISP(page.Size, &pageReadBack);
                if (ret == Status::Code::GOOD)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            
            if (page.Size != pageReadBack.Size)
            {
                LOG_ERROR(logger, "READ BACK PAGE SIZE DOES NOT MATCH: Target: %u, Actual: %u",
                    page.Size, pageReadBack.Size);
                return false;
            }

            for (size_t i = 0; i < page.Size; ++i)
            {
                if (page.Data[i] != pageReadBack.Data[i])
                {
                    LOG_ERROR(logger, "WRITTEN DATA DOES NOT MATCH: Target: %u, Actual: %u",
                        page.Data[i], pageReadBack.Data[i]);
                    return false;
                }
            }

            hexParser.RemovePage();
            /**
             * @brief 워드 주소 체계에 맞추기 위해 페이지 사이즈를 2로 나눕니다.
             */
            currentAddress += page.Size / 2;
            LOG_DEBUG(muffin::logger, "Page Remained: %u", hexParser.GetPageCount());
        }

        mega2560.LeaveProgrammingMode();
        mega2560.TearDown();
        LOG_INFO(logger, "ATmega2560 firmware has been updated");
        ret = catFS->Close();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO CLOSE FILE FROM CATFS");
            return false;
        }
        return true;
    }

    Status strategyATmega2560()
    {
        /**
         * @todo 결과 전송 실패 시 재전송하도록 코드를 수정해야 합니다.
         */
        Status ret = DownloadFirmware(mcu_type_e::MCU_ATmega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE FROM THE SERVER");
            PostDownloadResult(mcu_type_e::MCU_ATmega2560, "failure");
            return ret;
        }
        LOG_INFO(logger, "Succeded to download firmware from the server");
        PostDownloadResult(mcu_type_e::MCU_ATmega2560, "success");

        if (updateATmega2560() == true)
        {
            LOG_INFO(logger, "Succeded to update ATmega2560 firmware");
            PostFinishResult(mcu_type_e::MCU_ATmega2560, "success");
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ATmega2560 FIRMWARE");
            PostFinishResult(mcu_type_e::MCU_ATmega2560, "failure");
            return Status(Status::Code::BAD);
        }
    }
#endif
    void strategyESP32()
    {
        /**
         * @todo 결과 전송 실패 시 재전송하도록 코드를 수정해야 합니다.
         */
        Status ret = DownloadFirmware(mcu_type_e::MCU_ESP32);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE FROM THE SERVER");
            PostDownloadResult(mcu_type_e::MCU_ESP32, "failure");
            return;
        }
        LOG_INFO(logger, "Succeded to download firmware from the server");
        PostDownloadResult(mcu_type_e::MCU_ESP32, "success");

        if (updateESP32() == true)
        {
            LOG_INFO(logger, "Succeded to update ESP32 firmware");
            PostFinishResult(mcu_type_e::MCU_ESP32, "success");
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ESP32 FIRMWARE");
            PostFinishResult(mcu_type_e::MCU_ESP32, "failure");
        }
    }

    void implementUpdateTask()
    {
        new_fw_t fwInfo;
        Status ret = HasNewFirmware(&fwInfo);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE FIRMWARE INFO: %s", ret.c_str());
            return ;
        }

        if (fwInfo.MCU_ESP32 == false && fwInfo.MCU_MEGA2560 == false)
        {
            LOG_INFO(logger, "No Firmware To Update");
            return ;
        }

        LOG_INFO(logger, "Has New Firmware To Update");

        StopAllTask();

    #if defined(MODLINK_T2) || defined(MODLINK_B)
        if (fwInfo.MCU_MEGA2560 == true)
        {
            LOG_WARNING(logger,"START MEGA OTA");
            strategyATmega2560();
        }
    #endif
        if (fwInfo.MCU_ESP32 == true)
        {
            LOG_WARNING(logger,"START ESP OTA");
            strategyESP32();
        }

    #if defined(MODLINK_T2) || defined(MODLINK_B)
        spear.Reset();
    #endif
        ESP.restart();
    }

    void StartManualFirmwareUpdate(const std::string& payload)
    {
        new_fw_t fwInfo;
        Status ret = validateFirmwareInfo(payload, &fwInfo, true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID FIRMWARE INFO");
            return ;
        }
        
        if (fwInfo.MCU_ESP32 == false && fwInfo.MCU_MEGA2560 == false)
        {
            LOG_INFO(logger, "No Firmware To Update");
            return ;
        }
        LOG_INFO(logger, "Has New Firmware To Update");
        StopAllTask();
        
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        if (fwInfo.MCU_MEGA2560 == true)
        {
            strategyATmega2560();
        }
    #endif
        if (fwInfo.MCU_ESP32 == true)
        {
            strategyESP32();
        }

        LOG_INFO(logger, "Finish Firmware To Update Process, Reset!");
        delay(3000);
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        spear.Reset();
    #endif
        ESP.restart();
    }

    void StopAllTask()
    {
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        while (cdo.Count() > 0)
        {
            delay(100);
        }
    
        StopCyclicalsMSGTask();
        StopModbusTcpTask();
        StopModbusRtuTask();
    
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        alarmMonitor.StopTask();
        alarmMonitor.Clear();
        
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        productionInfo.StopTask();
        productionInfo.Clear();
        
        OperationTime& operationTime = OperationTime::GetInstance();
        operationTime.StopTask();
        operationTime.Clear();

        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        nodeStore->Clear();

        ModbusRtuVector.clear();
        ModbusTcpVector.clear();
    }
}