/**
 * @file UpdateTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-11-29
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include <Update.h>
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
#include "Core/Device/DeviceStatus.h"
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
#include "IM/MacAddress/MacAddress.h"
#include "Storage/CatFS/CatFS.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "IM/EA/DeprecableProductionInfo.h"



namespace muffin {

    fota_Info_t info;
    url_t DownloadURL;
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

    Status validateFirmwareInfo(const std::string& payload, new_fw_t* outInfo, const bool isManual)
    {
        JSON json;
        JsonDocument doc;
        Status ret = json.Deserialize(payload, &doc);
        if (ret != Status::Code::GOOD)
        {
            switch (ret.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                LOG_ERROR(logger,"PAYLOAD INSUFFICIENT OR INCOMPLETE");
                return ret;
            case Status::Code::BAD_NO_DATA:
                LOG_ERROR(logger,"PAYLOAD EMPTY");
                return ret;
            case Status::Code::BAD_DATA_ENCODING_INVALID:
                LOG_ERROR(logger,"PAYLOAD INVALID ENCODING");
                return ret;
            case Status::Code::BAD_OUT_OF_MEMORY:
                LOG_ERROR(logger,"PAYLOAD OUT OF MEMORY");
                return ret;
            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                LOG_ERROR(logger,"PAYLOAD EXCEEDED NESTING LIMIT");
                return ret;
            case Status::Code::BAD_UNEXPECTED_ERROR:
                LOG_ERROR(logger,"UNDEFINED CONDITION");
                return ret;
            default:
                LOG_ERROR(logger,"UNDEFINED CONDITION");
                return ret;
            }
        }

        bool isValid = true;
        isValid &= doc.containsKey("mac");
        isValid &= doc.containsKey("deviceType");
        isValid &= doc.containsKey("url");
        isValid &= doc.containsKey("otaId");
        isValid &= doc.containsKey("mcu1");
        isValid &= doc.containsKey("mcu2");
        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEYS CANNOT BE MISSING");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        if (doc["mac"].isNull() == true)
        {
            LOG_ERROR(logger, "MAC ADDRESS CANNOT BE NULL OR EMPTY");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        else if (strcmp(doc["mac"].as<const char*>(), MacAddress::GetEthernet()) != 0)
        {
            LOG_ERROR(logger, "THE MAC ADDRESS DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        isValid &= doc["deviceType"].isNull() == false;
        isValid &= doc["otaId"].isNull() == false;
        isValid &= doc["url"].isNull() == false;
        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

    #if defined(MODLINK_L)
        if (doc["deviceType"].as<std::string>() != "MODLINK-L")
        {
            LOG_ERROR(logger, "THE DEVICE TYPE DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    #elif defined(MODLINK_T2)
        if (doc["deviceType"].as<std::string>() != "MODLINK-T2")
        {
            LOG_ERROR(logger, "THE DEVICE TYPE DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    #endif

        if (doc["otaId"].is<uint32_t>() == false)
        {
            LOG_ERROR(logger, "INVALID OTA ID: %d", doc["otaId"].as<int64_t>());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        info.OtaID = doc["otaId"].as<uint32_t>();

        
        const std::string url = doc["url"].as<std::string>();
        size_t posHost = 0;
        if (url.substr(0, 7) == "http://")
        {
            LOG_INFO(logger, "OTA API Scheme: HTTP");
            DownloadURL.Scheme = http_scheme_e::HTTP;
            posHost = 7;
        }
        else if (url.substr(0, 8) == "https://")
        {
            LOG_INFO(logger, "OTA API Scheme: HTTPS");
            DownloadURL.Scheme = http_scheme_e::HTTPS;
            posHost = 8;
        }
        else
        {
            LOG_ERROR(logger, "INVALID PROTOCOL SCHEME: %s", url.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        
        size_t posPort = url.find(":", posHost);
        if (posPort == std::string::npos)
        {
            LOG_ERROR(logger, "INVALID DELIMITER FOR PORT: %s", url.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        DownloadURL.Host = url.substr(posHost, (posPort - posHost));
        DownloadURL.Port = Convert.ToUInt16(url.substr(posPort + 1));
        LOG_INFO(logger, "OTA Host: %s", DownloadURL.Host.c_str());
        LOG_INFO(logger, "OTA Port: %u", DownloadURL.Port);


        if (doc["mcu1"].isNull() == true)
        {
            LOG_INFO(logger, "No firmware available for ESP32");
        }
        else
        {
            if (doc["mcu1"].is<JsonObject>() == false)
            {
                LOG_ERROR(logger, "FIRMWARE INFO IS NOT A JSON OBJECT");
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }

            JsonObject obj = doc["mcu1"].as<JsonObject>();
            info.mcu1.VersionCode = obj["vc"].as<uint16_t>();
            info.mcu1.FirmwareVersion = obj["version"].as<std::string>();
            info.mcu1.FileTotalSize = obj["fileTotalSize"].as<uint64_t>();
            info.mcu1.FileTotalChecksum = obj["checksum"].as<std::string>();

            DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
            const fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu_type_e::MCU_ESP32);
            if (version.Code < info.mcu1.VersionCode || isManual == true)
            {
                outInfo->MCU_ESP32 = true;
                LOG_INFO(logger, "New firmware for ESP32 with VC: %u", info.mcu1.VersionCode);

                info.mcu1.FileNumberVector.clear();
                info.mcu1.FilePathVector.clear();
                info.mcu1.FileSizeVector.clear();
                info.mcu1.FileChecksumVector.clear();

                for (auto num : obj["fileNo"].as<JsonArray>())
                {
                    info.mcu1.FileNumberVector.emplace_back(num.as<uint8_t>());
                }

                for (auto path : obj["filePath"].as<JsonArray>())
                {
                    info.mcu1.FilePathVector.emplace_back(path.as<std::string>());
                }
                
                for (auto size : obj["fileSize"].as<JsonArray>())
                {
                    info.mcu1.FileSizeVector.emplace_back(size.as<std::uint32_t>());
                }

                for (auto cks : obj["fileChecksum"].as<JsonArray>())
                {
                    info.mcu1.FileChecksumVector.emplace_back(cks.as<std::string>());
                }
            }
            else
            {
                outInfo->MCU_ESP32 = false;
                LOG_INFO(logger, "No update for ESP32");
            }
        }


        if (doc["mcu2"].isNull() == true)
        {
            LOG_INFO(logger, "No firmware available for ATmega2560");
        }
        else
        {
            if (doc["mcu2"].is<JsonObject>() == false)
            {
                LOG_ERROR(logger, "FIRMWARE INFO IS NOT A JSON OBJECT");
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }

            JsonObject obj = doc["mcu2"].as<JsonObject>();
            info.mcu2.VersionCode = obj["vc"].as<uint16_t>();
            info.mcu2.FirmwareVersion = obj["version"].as<std::string>();
            info.mcu2.FileTotalSize = obj["fileTotalSize"].as<uint64_t>();
            info.mcu2.FileTotalChecksum = obj["checksum"].as<std::string>();

            DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
            const fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu_type_e::MCU_ATmega2560);
            if (version.Code < info.mcu2.VersionCode || isManual == true)
            {
                outInfo->MCU_MEGA2560 = true;
                LOG_INFO(logger, "New firmware for ATmega2560 with VC: %u", info.mcu1.VersionCode);

                info.mcu2.FileNumberVector.clear();
                info.mcu2.FilePathVector.clear();
                info.mcu2.FileSizeVector.clear();
                info.mcu2.FileChecksumVector.clear();

                for (auto num : obj["fileNo"].as<JsonArray>())
                {
                    info.mcu2.FileNumberVector.emplace_back(num.as<uint8_t>());
                }

                for (auto path : obj["filePath"].as<JsonArray>())
                {
                    info.mcu2.FilePathVector.emplace_back(path.as<std::string>());
                }
                
                for (auto size : obj["fileSize"].as<JsonArray>())
                {
                    info.mcu2.FileSizeVector.emplace_back(size.as<std::uint32_t>());
                }

                for (auto cks : obj["fileChecksum"].as<JsonArray>())
                {
                    info.mcu2.FileChecksumVector.emplace_back(cks.as<std::string>());
                }
            }
            else
            {
                outInfo->MCU_MEGA2560 = false;
                LOG_INFO(logger, "No update for ATmega2560");
            }
        }

        return Status(Status::Code::GOOD);
    }

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
            /* code */
            return false;
        }
        LOG_INFO(logger,"info.mcu1.FileSizeVector.at(0) : %u", info.mcu1.FileSizeVector.at(0));

        ASSERT((info.mcu1.FileSizeVector.size() != 0),"FILE SIZE VECTOR MUST HAS VALUE");

        if ( Update.begin(info.mcu1.FileSizeVector.at(0)) == false )
        {
            LOG_ERROR(logger,"Can't begin OTA since the file size is bigger than the memory");
            return false;
        }

        uint32_t startMillis = millis();
        uint32_t writtenSize = 0;
        while ( writtenSize < info.mcu1.FileSizeVector.at(0) )
        {
            uint16_t length = 4096;

            if ( (info.mcu1.FileSizeVector.at(0) - writtenSize) < 4096 )
            {
                length = info.mcu1.FileSizeVector.at(0) - writtenSize;
            }
            
            uint8_t bytes[length] = {0};
            catFS->Read(length, bytes);
            writtenSize += Update.write(bytes, length);

            LOG_DEBUG(logger ,"Written %d bytes", writtenSize);
        }
        uint32_t finishMillis = millis();
        LOG_INFO(logger ,"\n\nProcessing time : %d \n\n", finishMillis - startMillis);

        Serial.print("Update Error : ");
        Update.printError(Serial);
        Serial.print("\n");

        bool CheckUpdate = Update.end();
        bool CheckFinished = Update.isFinished();
        LOG_DEBUG(logger, "Update.end() : %s", CheckUpdate ? "true" : "false");
        LOG_DEBUG(logger, "Update.isFinished() : %s", CheckFinished ? "true" : "false");

        if (CheckFinished  == false || CheckUpdate == false)
        {
            return false;
        }
        
        return true;
    }

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
            mega2560.LoadAddress(currentAddress);
            mega2560.ReadFlashISP(page.Size, &pageReadBack);
            
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
            ESP.restart();
        }
        LOG_INFO(logger, "Succeded to download firmware from the server");
        PostDownloadResult(mcu_type_e::MCU_ESP32, "success");

        if (updateESP32() == true)
        {
            LOG_INFO(logger, "Succeded to update ESP32 firmware");
            PostFinishResult(mcu_type_e::MCU_ESP32, "success");
            ESP.restart();
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ESP32 FIRMWARE");
            PostFinishResult(mcu_type_e::MCU_ESP32, "failure");
            ESP.restart();
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

        if (fwInfo.MCU_MEGA2560 == true)
        {
            LOG_WARNING(logger,"START MEGA OTA");
            strategyATmega2560();
        }

        if (fwInfo.MCU_ESP32 == true)
        {
            LOG_WARNING(logger,"START ESP OTA");
            strategyESP32();
        }
        
    }

    void UpdateTask(void* pvParameter)
    {
        const uint16_t fwCheckInterval = 3600 * 12;
        const uint32_t sleep10Minutes = 60 * 10 * 1000;
        time_t currentTimestamp = GetTimestamp();

        implementUpdateTask();

        while (true)
        {
            if (GetTimestamp() - currentTimestamp < fwCheckInterval)
            {
                vTaskDelay(sleep10Minutes / portTICK_PERIOD_MS); 
                continue;
            }
            currentTimestamp = GetTimestamp();
            LOG_DEBUG(logger,"12시간 경과 %lu",currentTimestamp);
        #ifdef DEBUG
            LOG_DEBUG(logger, "[TASK: Fota] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
        #endif
            implementUpdateTask();
        }
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
            http_scheme_e::HTTPS, 
            ReleaseURL.Host, 
            ReleaseURL.Port, 
            "/firmware/file/version/release",
        #if defined(MODLINK_T)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
        http::RequestParameter parameters;
        parameters.Add("mac", MacAddress::GetEthernet());
        
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
        #if defined(MODLINK_T)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
        
        http::RequestParameter parameters;
        parameters.Add("mac", MacAddress::GetEthernet());
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
        Status ret = catHttp.GET(mutexHandle.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catHttp.SetSinkToCatFS(false,"");
            catM1.ReleaseMutex();
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        catHttp.SetSinkToCatFS(false,"");
        catM1.ReleaseMutex();
        
        
        CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
        ret = catFS->Begin();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO BEGIN CATFS");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        ret = catFS->Open(path, true);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAIL TO OPEN FILE FROM CATFS");
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
            
            uint8_t buffer[bufferSize] = { 0 };
            Status ret = catFS->Read(bufferSize, buffer);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", ret.c_str());
                catFS->Close();
                return ret;
            }
            bytesRead += bufferSize;
            crc32.Calculate(bufferSize, buffer);
        }
        catFS->Close();

        crc32.Teardown();
        char bufferCRC32[9] = { '\0' };
        sprintf(bufferCRC32, "%08x", crc32.RetrieveTotalChecksum());
        const std::string calculatedCRC32(bufferCRC32);
        LOG_DEBUG(logger, "Calculated CRC32: %s", calculatedCRC32.c_str());

        const std::string receivedCRC32 = mcu == mcu_type_e::MCU_ESP32 ? 
            info.mcu1.FileTotalChecksum :
            info.mcu2.FileTotalChecksum;
        LOG_DEBUG(logger, "Received CRC32: %s", receivedCRC32.c_str());

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
        #if defined(MODLINK_T)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
    
        http::RequestBody body("application/x-www-form-urlencoded");
        body.AddProperty("mac", MacAddress::GetEthernet());
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
        #if defined(MODLINK_T)
            "MODLINK-L/" + version.Semantic
        #elif defined(MODLINK_T2)
            "MODLINK-T2/" + version.Semantic
        #endif
        );
    

        http::RequestParameter parameters;
        parameters.Add("mac", MacAddress::GetEthernet());
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

        if (fwInfo.MCU_MEGA2560 == true)
        {
            strategyATmega2560();
        }

        if (fwInfo.MCU_ESP32 == true)
        {
            strategyESP32();
        }
    }

    void SendStatusMSG()
    {
        DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
        fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu_type_e::MCU_ESP32);

        fota_status_t status;
        status.VersionCodeMcu1  = version.Code;
        status.VersionMcu1      = version.Semantic;
    #if defined(MODLINK_T2)
        fw_vsn_t versionATmega2560 = deviceStatus.GetFirmwareVersion(mcu_type_e::MCU_ATmega2560);
        status.VersionCodeMcu2  = versionATmega2560.Code;
        status.VersionMcu2      = versionATmega2560.Semantic;
    #endif

        /**
         * @todo ATmega2560에 대한 정보도 전송할 수 있도록 코드 수정이 필요합니다.
         */

        JSON json;
        std::string payload = json.Serialize(status);
        mqtt::Message message(mqtt::topic_e::FOTA_STATUS, payload);
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        LOG_INFO(logger, "FOTA/STAUTS PAYLOAD : %s",payload.c_str());
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

    void StartUpdateTask()
    {
        if (xTaskFotaHandle != NULL)
        {
            LOG_WARNING(logger, "THE OTA TASK HAS ALREADY STARTED");
            return;
        }
        SendStatusMSG();
        delay(1000);

        /**
         * @todo 스택 오버플로우를 방지하기 위해서 MQTT 메시지 크기에 따라서
         *       태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            UpdateTask,      // Function to be run inside of the task
            "UpdateTask",    // The identifier of this task for men
            10240,			   // Stack memory size to allocate
            NULL,			   // Task parameters to be passed to the function
            0,				   // Task Priority for scheduling
            &xTaskFotaHandle,  // The identifier of this task for machines
            0				   // Index of MCU core where the function to run
        );
        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The FOTA task has been started");
            // return Status(Status::Code::GOOD);
            return;

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            return;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
            return;

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            return;
        }
    }
}