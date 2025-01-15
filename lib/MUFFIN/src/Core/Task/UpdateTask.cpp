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
#include "UpdateTask.h"
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
#if defined(DEBUG)
    url_t ReleaseURL =
    {
        .Scheme = http_scheme_e::HTTP,
        .Host = "112.171.127.186",
        .Port = 8123
    };
#else
    url_t ReleaseURL =
    {
        .Scheme = http_scheme_e::HTTPS,
        .Host = "api.fota.edgecross.ai",
        .Port = 443
    };
#endif
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

    Status HasNewFirmware(new_fw_t* outInfo)
    {
        ASSERT((outInfo != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");


        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        }

        DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
        fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu_type_e::MCU_ESP32);
        
        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(
            rest_method_e::GET, 
            ReleaseURL.Scheme, 
            ReleaseURL.Host, 
            ReleaseURL.Port, 
            "/firmware/file/version/release",
        #if defined(MODLINK_L)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        
        Status ret = catHttp.GET(mutexHandle.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();
            return ret;
        }

        std::string payload;
        ret = catHttp.Retrieve(mutexHandle.second, &payload);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
            catM1.ReleaseMutex();
            return ret;
        }
        catM1.ReleaseMutex();
        LOG_INFO(logger, "Firmware Version Info\n%s", payload.c_str());
        return validateFirmwareInfo(payload, outInfo, false);
    }

    Status DownloadFirmware(const mcu_type_e mcu)
    {
        CatM1& catM1 = CatM1::GetInstance();
        catM1.KillUrcTask(true);
        
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        }

        DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
        fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu);

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(
            rest_method_e::GET, 
            DownloadURL.Scheme,
            DownloadURL.Host,
            DownloadURL.Port,
            "/firmware/file/download", 
        #if defined(MODLINK_L)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
        
        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(info.OtaID));
        if (mcu == mcu_type_e::MCU_ESP32)
        {
            parameters.Add("mcu1.vc", std::to_string(info.mcu1.VersionCode));
            parameters.Add("mcu1.version", info.mcu1.FirmwareVersion);
            parameters.Add("mcu1.fileNo", std::to_string(info.mcu1.FileNumberVector.at(0)));
            parameters.Add("mcu1.filepath", info.mcu1.FilePathVector.at(0));
        }
        else
        {
            parameters.Add("mcu2.vc", std::to_string(info.mcu2.VersionCode));
            parameters.Add("mcu2.version", info.mcu2.FirmwareVersion);
            parameters.Add("mcu2.fileNo", std::to_string(info.mcu2.FileNumberVector.at(0)));
            parameters.Add("mcu2.filepath", info.mcu2.FilePathVector.at(0));
        }
        
        /**
         * @todo Chunk 방식이 정착되면 UART로 직접 받도록 수정이 필요함
         */
        std::string path =  mcu == mcu_type_e::MCU_ESP32 ? "esp32" : "mega2560";
        catHttp.SetSinkToCatFS(true, path);
        Status ret = catHttp.GET(mutexHandle.second, header, parameters, 300);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catHttp.SetSinkToCatFS(false, "");
            catM1.ReleaseMutex();
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        catHttp.SetSinkToCatFS(false, "");
        
        
        CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
        ret = catFS->BeginWithMutex(mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO BEGIN CATFS");
            catM1.ReleaseMutex();
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        ret = catFS->OpenWithMutex(mutexHandle.second, path, true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO OPEN FILE FROM CATFS");
            catM1.ReleaseMutex();
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

/**
 * @todo 청크 방식으로 변경 시 객체를 호출자 내부로 옮겨야 합니다.
 *       그래야 누적된 CRC32 값을 구하여 검증에 사용할 수 있습니다.
 */
        CRC32 crc32;
        crc32.Init();
        
        size_t bytesRead = 0;
        size_t fileSize = mcu == mcu_type_e::MCU_ESP32 ? 
            info.mcu1.FileSizeVector.at(0) :
            info.mcu2.FileSizeVector.at(0);
        size_t maxBufferSize = 4 * 1024;

        while (bytesRead < fileSize)
        {
            const size_t bytesRemained = fileSize - bytesRead;
            const size_t bufferSize = bytesRemained > maxBufferSize ?  
                maxBufferSize :
                bytesRemained;
            
            LOG_DEBUG(logger, "File: %u, ByteRead: %u, BufferSize: %u", fileSize, bytesRead, bufferSize);
            
            uint8_t buffer[bufferSize] = { 0 };
            Status ret = catFS->ReadWithMutex(mutexHandle.second, bufferSize, buffer);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", ret.c_str());
                catFS->CloseWithMutex(mutexHandle.second);
                catM1.ReleaseMutex();
                return ret;
            }
            bytesRead += bufferSize;
            crc32.Calculate(bufferSize, buffer);
        }
        catFS->CloseWithMutex(mutexHandle.second);
        catM1.ReleaseMutex();

        crc32.Teardown();
        char bufferCRC32[9] = { '\0' };
        sprintf(bufferCRC32, "%08x", crc32.RetrieveTotalChecksum());
        const std::string calculatedCRC32(bufferCRC32);
        LOG_WARNING(logger, "Calculated CRC32: %s", calculatedCRC32.c_str());

        const std::string receivedCRC32 = mcu == mcu_type_e::MCU_ESP32 ? 
            info.mcu1.FileTotalChecksum :
            info.mcu2.FileTotalChecksum;
        LOG_WARNING(logger, "Received CRC32: %s", receivedCRC32.c_str());

        if (bytesRead != fileSize)
        {
            LOG_ERROR(logger, "FILE SIZE DOES NOT MATCH: TARGET: %u, ACTUAL: %u",
                fileSize, bytesRead);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        
        if (calculatedCRC32 != receivedCRC32)
        {
            LOG_ERROR(logger, "CRC32 DOES NOT MATCH: TARGET: %s, ACTUAL: %s",
                receivedCRC32.c_str(), calculatedCRC32.c_str());
            return Status(Status::Code::BAD_DATA_LOST);
        }

        LOG_INFO(logger, "Succeded to download firmware from the server");
        return ret;
    }

    bool PostDownloadResult(const mcu_type_e mcu, const std::string result)
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
        fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu);

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(
            rest_method_e::POST,
            http_scheme_e::HTTPS,
            DownloadURL.Host,
            DownloadURL.Port,
            "/firmware/file/download",
        #if defined(MODLINK_L)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
    
        http::RequestBody body("application/x-www-form-urlencoded");
        body.AddProperty("mac", macAddress.GetEthernet());
        body.AddProperty("otaId", std::to_string(info.OtaID));
        if (mcu == mcu_type_e::MCU_ESP32)
        {
            body.AddProperty("mcu1.vc", std::to_string(info.mcu1.VersionCode));
            body.AddProperty("mcu1.version", info.mcu1.FirmwareVersion);
            body.AddProperty("mcu1.fileNo", std::to_string(info.mcu1.FileNumberVector.at(0)));
            body.AddProperty("mcu1.filepath", info.mcu1.FilePathVector.at(0));
            body.AddProperty("mcu1.result", result);
        }
        else
        {
            body.AddProperty("mcu2.vc", std::to_string(info.mcu2.VersionCode));
            body.AddProperty("mcu2.version", info.mcu2.FirmwareVersion);
            body.AddProperty("mcu2.fileNo", std::to_string(info.mcu2.FileNumberVector.at(0)));
            body.AddProperty("mcu2.filepath", info.mcu2.FilePathVector.at(0));
            body.AddProperty("mcu2.result", result);
        }


        Status ret = catHttp.POST(mutexHandle.second, header, body);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();
            return false;
        }
        LOG_INFO(logger, "Succeded to POST download result");
        catM1.ReleaseMutex();
        return true;
    }

    bool PostFinishResult(const mcu_type_e mcu, const std::string result)
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
        fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu);

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(
            rest_method_e::POST, 
            http_scheme_e::HTTPS, 
            DownloadURL.Host, 
            DownloadURL.Port, 
            "/firmware/file/download/finish", 
        #if defined(MODLINK_L)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
    

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(info.OtaID));
        if (mcu == mcu_type_e::MCU_ESP32)
        {
            parameters.Add("mcu1.vc", std::to_string(info.mcu1.VersionCode));
            parameters.Add("mcu1.version", info.mcu1.FirmwareVersion);
            parameters.Add("mcu1.result", result);
        }
        else
        {
            parameters.Add("mcu2.vc", std::to_string(info.mcu2.VersionCode));
            parameters.Add("mcu2.version", info.mcu2.FirmwareVersion);
            parameters.Add("mcu2.result", result);
        }

        Status ret = catHttp.POST(mutexHandle.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();
            return false;
        }
        
        catM1.ReleaseMutex();
        return true;
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

    void SendStatusMSG()
    {
        fota_status_t status;
        status.VersionCodeMcu1  = FW_VERSION_ESP32.GetVersionCode();
        status.VersionMcu1      = FW_VERSION_ESP32.GetSemanticVersion();
    #if defined(MODLINK_T2)
        status.VersionCodeMcu2  = FW_VERSION_MEGA2560.GetVersionCode();
        status.VersionMcu2      = FW_VERSION_MEGA2560.GetSemanticVersion();
    #endif

        /**
         * @todo ATmega2560에 대한 정보도 전송할 수 있도록 코드 수정이 필요합니다.
         */
        constexpr size_t size = 256;
        char buffer[size] = {'\0'};

        JSON json;
        json.Serialize(status, size, buffer);
        LOG_INFO(logger, "[Topic] fota/status: %s", buffer);

        mqtt::Message message(mqtt::topic_e::FOTA_STATUS, buffer);
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        Status ret = cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
            /**
             * @todo Store 실패시 flash 메모리에 저장하는 것과 같은 방법을 적용하여 실패에 강건하도록 코드를 작성해야 합니다.
             */
            LOG_ERROR(logger, "FAIL TO STORE JARVIS RESPONSE MESSAGE INTO CDO: %s", ret.c_str());
            return;
        }
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