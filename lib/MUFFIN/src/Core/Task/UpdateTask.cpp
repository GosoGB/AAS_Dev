/**
 * @file UpdateTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-10-29
 * @version 1.0.0
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
#include "Core/Core.h"
#include "CyclicalPubTask.h"
#include "UpdateTask.h"
#include "ModbusTask.h"
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
    Fota_url_t DownloadUrl;
    Fota_url_t ReleaseUrl =
    {
        .Port = 443,
        .Host = "api.fota.edgecross.ai"
    };

    TaskHandle_t xTaskFotaHandle = NULL;

    void StartUpdateTask()
    {
        if (xTaskFotaHandle != NULL)
        {
            LOG_WARNING(logger, "THE FOTA TASK HAS ALREADY STARTED");
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

    void UpdateTask(void* pvParameter)
    {   
        bool result;
        result = HasNewFirmwareFOTA();
        if (result == false)
        {
            goto NO_OTA;
        }

        LOG_INFO(logger, "NEW FIRMWARE EXIST!!! currnet : %u, server : %u",FIRMWARE_VERSION_CODE_MCU1, info.mcu1.VersionCode);
        StopAllTask();
        result = DownloadFirmware();
        if (result == true)
        {
            LOG_INFO(logger, "Firmware Download Succsess");
            result = PostDownloadResult("success");
        }
        else
        {
            LOG_ERROR(logger, "FAIL TO FIRMWARE DOWNLOAD");
            result = PostDownloadResult("failure");
            ESP.restart();
        }

        if (result == true)
        {
            LOG_DEBUG(logger,"POST Method Succsess");
            bool resultOTA = UpdateFirmware();
            if ( resultOTA)
            {
                if(PostFinishResult("success"))
                {
                    ESP.restart();
                }
            }
            else
            {
                if(PostFinishResult("failure"))
                {
                    ESP.restart();
                }
                
            }
        }
        
    NO_OTA:  
        time_t currentTimestamp = GetTimestamp();
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        while (true)
        {
            if (GetTimestamp() - currentTimestamp < 3600 * 12 )
            {
                vTaskDelay(100 / portTICK_PERIOD_MS); 
                continue;
            }
            currentTimestamp = GetTimestamp();

            LOG_DEBUG(logger,"12시간 경과 %lu",currentTimestamp);
            result = HasNewFirmwareFOTA();
            if (result == false)
            {
                goto NO_OTA_IN_TASK;
            }

            LOG_INFO(logger, "NEW FIRMWARE EXIST!!! currnet : %u, server : %u",FIRMWARE_VERSION_CODE_MCU1, info.mcu1.VersionCode);
            StopAllTask();
            result = DownloadFirmware();
            if (result == true)
            {
                LOG_INFO(logger, "Firmware Download Succsess");
                result = PostDownloadResult("success");
            }
            else
            {
                LOG_ERROR(logger, "FAIL TO FIRMWARE DOWNLOAD");
                result = PostDownloadResult("failure");
                ESP.restart();
            }

            if (result == true)
            {
                LOG_DEBUG(logger,"POST Method Succsess");
                bool resultOTA = UpdateFirmware();
                if ( resultOTA)
                {
                    if(PostFinishResult("success"))
                    {
                        ESP.restart();
                    }
                }
                else
                {
                    if(PostFinishResult("failure"))
                    {
                        ESP.restart();
                    }
                }
            }
        NO_OTA_IN_TASK:

            vTaskDelay(100 / portTICK_PERIOD_MS); 
        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "[TASK: Fota] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }

    bool HasNewFirmwareFOTA()
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, ReleaseUrl.Host, ReleaseUrl.Port, "/firmware/file/version/release", "MODLINK-L/1.0.0");
        http::RequestParameter parameters;
        parameters.Add("mac", MacAddress::GetEthernet());
        
        Status ret = catHttp.GET(mutexHandle.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();
            return false;
        }
        std::string payload;

        ret = catHttp.Retrieve(mutexHandle.second, &payload);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
            catM1.ReleaseMutex();
            return false;
        }

        LOG_INFO(logger, "RECEIVED FOTA: %s", payload.c_str());

        catM1.ReleaseMutex();
        JsonDocument doc;
        JSON json;
        Status retJSON = json.Deserialize(payload, &doc);
        if (retJSON != Status::Code::GOOD)
        {
            switch (retJSON.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                LOG_ERROR(logger,"PAYLOAD INSUFFICIENT OR INCOMPLETE");
                return false;
            case Status::Code::BAD_NO_DATA:
                LOG_ERROR(logger,"PAYLOAD EMPTY");
                return false;
            case Status::Code::BAD_DATA_ENCODING_INVALID:
                LOG_ERROR(logger,"PAYLOAD INVALID ENCODING");
                return false;
            case Status::Code::BAD_OUT_OF_MEMORY:
                LOG_ERROR(logger,"PAYLOAD OUT OF MEMORY");
                return false;
            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                LOG_ERROR(logger,"PAYLOAD EXCEEDED NESTING LIMIT");
                return false;
            case Status::Code::BAD_UNEXPECTED_ERROR:
                LOG_ERROR(logger,"UNDEFINED CONDITION");
                return false;
            default:
                LOG_ERROR(logger,"UNDEFINED CONDITION");
                return false;
            }
        }
        
        /**
         * @todo Validator 부분 만들어야 함!!!!!!!!!!
         */
        bool isValid = true;
        isValid &= doc.containsKey("mac");
        isValid &= doc.containsKey("deviceType");
        isValid &= doc.containsKey("otaId");
        isValid &= doc.containsKey("mcu1");

        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY CANNOT BE MISSING");
            return false;
        }

        isValid &= doc["otaId"].isNull() == false;
        isValid &= doc["mcu1"].isNull() == false;
        isValid &= doc["otaId"].is<uint8_t>();
        isValid &= doc["mcu1"].is<JsonObject>();

        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return false;
        }

        if (doc["url"].isNull() == false)
        {
            std::string url = doc["url"].as<std::string>();

            size_t pos = url.find("://");
            if (pos != std::string::npos) 
            {
                url = url.substr(pos + 3);
            }
            pos = url.find(":");

            if (pos != std::string::npos) 
            {
                DownloadUrl.Host = url.substr(0, pos);
                DownloadUrl.Port = static_cast<uint16_t>(atoi(url.substr(pos + 1).c_str())); 
            }
        }

        
        info.OtaID = doc["otaId"].as<uint8_t>();

        JsonObject obj = doc["mcu1"].as<JsonObject>();
        info.mcu1.VersionCode = obj["vc"].as<uint16_t>();
        info.mcu1.FirmwareVersion = obj["version"].as<std::string>();
        info.mcu1.FileTotalSize = obj["fileTotalSize"].as<uint64_t>();
        info.mcu1.FileTotalChecksum = obj["checksum"].as<std::string>();

        info.mcu1.FileNumberVector.clear();
        info.mcu1.FilePathVector.clear();
        info.mcu1.FileSizeVector.clear();
        info.mcu1.FileChecksumVector.clear();

        ASSERT((info.mcu1.FileNumberVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FilePathVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FileSizeVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FileChecksumVector.size() == 0) , "ALL DATA MUST BE EMBTY");

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

        if (doc["mcu2"].isNull() == false)
        {
            // MCU2 설정 값 저장
        }
        
        if (FIRMWARE_VERSION_CODE_MCU1 < info.mcu1.VersionCode)
        {
            return true;
        }

        return false;
    }


    bool DownloadFirmware()
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, DownloadUrl.Host, DownloadUrl.Port, "/firmware/file/download", "MODLINK-L/1.0.0");
        http::RequestParameter parameters;
        parameters.Add("mac", MacAddress::GetEthernet());
        parameters.Add("otaId", std::to_string(info.OtaID));
        parameters.Add("mcu1.vc", std::to_string(info.mcu1.VersionCode));
        parameters.Add("mcu1.version", info.mcu1.FirmwareVersion);
        parameters.Add("mcu1.fileNo", std::to_string(info.mcu1.FileNumberVector.at(0)));
        parameters.Add("mcu1.filepath", info.mcu1.FilePathVector.at(0));

        catHttp.SetSinkToCatFS(true);
        Status ret = catHttp.GET(mutexHandle.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catHttp.SetSinkToCatFS(false);
            catM1.ReleaseMutex();

            return false;
        }

        catHttp.SetSinkToCatFS(false);
        catM1.ReleaseMutex();
        return true;
    }

    bool PostFinishResult(const std::string resultStr)
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::POST, http_scheme_e::HTTPS, DownloadUrl.Host, DownloadUrl.Port, "/firmware/file/download/finish", "MODLINK-L/1.0.0");
        http::RequestBody body("application/x-www-form-urlencoded");
    
        body.AddProperty("mac", MacAddress::GetEthernet());
        body.AddProperty("otaId", std::to_string(info.OtaID));
        body.AddProperty("mcu1.vc", std::to_string(info.mcu1.VersionCode));
        body.AddProperty("mcu1.version", info.mcu1.FirmwareVersion);
        body.AddProperty("mcu1.result", resultStr);

        Status ret = catHttp.POST(mutexHandle.second, header, body);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();

           return false;
        }
        
        catM1.ReleaseMutex();
        return true;
    }

    bool PostDownloadResult(const std::string resultStr)
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::POST, http_scheme_e::HTTPS, DownloadUrl.Host, DownloadUrl.Port, "/firmware/file/download", "MODLINK-L/1.0.0");
        http::RequestBody body("application/x-www-form-urlencoded");
    
        body.AddProperty("mac", MacAddress::GetEthernet());
        body.AddProperty("otaId", std::to_string(info.OtaID));
        body.AddProperty("mcu1.vc", std::to_string(info.mcu1.VersionCode));
        body.AddProperty("mcu1.version", info.mcu1.FirmwareVersion);
        body.AddProperty("mcu1.fileNo", std::to_string(info.mcu1.FileNumberVector.at(0)));
        body.AddProperty("mcu1.filepath", info.mcu1.FilePathVector.at(0));
        body.AddProperty("mcu1.result", resultStr);

        Status ret = catHttp.POST(mutexHandle.second, header, body);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();

           return false;
        }
        
        catM1.ReleaseMutex();
        return true;
    }

    bool UpdateFirmware()
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
        
        ret = catFS->Open("http_response_file", true);
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

    void StartManualFirmwareUpdate(JsonDocument& doc)
    {
        /**
         * @todo Validator 부분 만들어야 함!!!!!!!!!!
         */
        bool isValid = true;
        isValid &= doc.containsKey("mac");
        isValid &= doc.containsKey("deviceType");
        isValid &= doc.containsKey("otaId");
        isValid &= doc.containsKey("mcu1");

        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY CANNOT BE MISSING");
            return;
        }

        isValid &= doc["otaId"].isNull() == false;
        isValid &= doc["mcu1"].isNull() == false;
        isValid &= doc["otaId"].is<uint8_t>();
        isValid &= doc["mcu1"].is<JsonObject>();

        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return;
        }
        
        if (doc["url"].isNull() == false)
        {
            std::string url = doc["url"].as<std::string>();

            size_t pos = url.find("://");
            if (pos != std::string::npos) 
            {
                url = url.substr(pos + 3);
            }
            pos = url.find(":");

            if (pos != std::string::npos) 
            {
                DownloadUrl.Host = url.substr(0, pos);
                DownloadUrl.Port = static_cast<uint16_t>(atoi(url.substr(pos + 1).c_str())); 
            }
        }
        

        info.OtaID = doc["otaId"].as<uint8_t>();
        

        JsonObject obj = doc["mcu1"].as<JsonObject>();
        info.mcu1.VersionCode = obj["vc"].as<uint16_t>();
        info.mcu1.FirmwareVersion = obj["version"].as<std::string>();
        info.mcu1.FileTotalSize = obj["fileTotalSize"].as<uint64_t>();
        info.mcu1.FileTotalChecksum = obj["checksum"].as<std::string>();
        
        info.mcu1.FileNumberVector.clear();
        info.mcu1.FilePathVector.clear();
        info.mcu1.FileSizeVector.clear();
        info.mcu1.FileChecksumVector.clear();
        
        ASSERT((info.mcu1.FileNumberVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FilePathVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FileSizeVector.size() == 0) , "ALL DATA MUST BE EMBTY");
        ASSERT((info.mcu1.FileChecksumVector.size() == 0) , "ALL DATA MUST BE EMBTY");
   
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

        if (doc["mcu2"].isNull() == false)
        {
            // MCU2 설정 값 저장
        }

        bool result;
        StopAllTask();
        result = DownloadFirmware();
        if (result == true)
        {
            LOG_INFO(logger, "Firmware Download Succsess");
            result = PostDownloadResult("success");
        }
        else
        {
            LOG_ERROR(logger, "FAIL TO FIRMWARE DOWNLOAD");
            PostDownloadResult("failure");
            return;
        }

        if (result == true)
        {
            LOG_DEBUG(logger,"POST Method Succsess");
            bool resultOTA = UpdateFirmware();
            if ( resultOTA)
            {
                if(PostFinishResult("success"))
                {
                    ESP.restart();
                }
            }
            else
            {
                PostFinishResult("failure");
                ESP.restart();
            }
        }
    }

    void SendStatusMSG()
    {
        fota_status_t status;

        status.VersionCodeMcu1 = FIRMWARE_VERSION_CODE_MCU1;
        status.VersionMcu1 = FIRMWARE_VERSION_MCU1;

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
        StopModbusTask();
        
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        alarmMonitor.StopTask();
        alarmMonitor.Clear();
        
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        productionInfo.StopTask();
        productionInfo.Clear();
        
        OperationTime& operationTime = OperationTime::GetInstance();
        operationTime.StopTask();
        operationTime.Clear();

        ModbusRTU* modbusRTU = ModbusRTU::CreateInstanceOrNULL();
        modbusRTU->Clear();

        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        nodeStore->Clear();
        
    }
}