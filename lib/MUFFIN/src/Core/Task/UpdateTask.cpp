/**
 * @file UpdateTask.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-10-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




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
#include "UpdateTask.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "IM/MacAddress/MacAddress.h"



namespace muffin {

    fota_Info_t info;
    std::string FotaHost;
    uint16_t FotaPort;

    TaskHandle_t xTaskFotaHandle = NULL;

    void StartUpdateTask()
    {
        if (xTaskFotaHandle != NULL)
        {
            LOG_WARNING(logger, "THE FOTA TASK HAS ALREADY STARTED");
            return;
        }

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
        LOG_DEBUG(logger,"1회 실행");
        bool result = HasNewFirmwareFOTA();
        if (result)
        {
            LOG_INFO(logger, "NEW FIRMWARE EXIST!!! currnet : %u, server : %u",FIRMWARE_VERSION, info.mcu1.VersionCode);
            result = DownloadFirmware();
            if (result)
            {
                LOG_INFO(logger, "Firmware Download Succsess");
                for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
                {
                    if (PostDownloadResult())
                    {
                        LOG_DEBUG(logger,"POST Method Succsess");
                        break;
                    }
                    else
                    {
                        LOG_WARNING(logger, "[TRIAL: #%u] FAIL TO POST METHOD ", i);
                    }
                    vTaskDelay((500) / portTICK_PERIOD_MS);
                }
                
            }
            else
            {
                LOG_INFO(logger, "FAIL TO FIRMWARE DOWNLOAD");
            }
            
        }
        

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

            LOG_WARNING(logger,"12시간 경과 %lu",currentTimestamp);
            bool result = HasNewFirmwareFOTA();
            if (result)
            {
                LOG_INFO(logger, "NEW FIRMWARE EXIST!!! currnet : %u, server : %u",FIRMWARE_VERSION, info.mcu1.VersionCode);
                DownloadFirmware();
            }


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
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTP, FotaHost, FotaPort, "/firmware/file/version/release", "MODLINK-L/0.0.1");
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

        
        info.OtaID = doc["otaId"].as<uint8_t>();

        JsonObject obj = doc["mcu1"].as<JsonObject>();
        info.mcu1.VersionCode = obj["vc"].as<uint16_t>();
        info.mcu1.FirmwareVersion = obj["version"].as<std::string>();
        info.mcu1.FileTotalSize = obj["fileTotalSize"].as<uint64_t>();
        info.mcu1.FileTotalChecksum = obj["checksum"].as<std::string>();

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
        
        if (FIRMWARE_VERSION < info.mcu1.VersionCode)
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
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTP, FotaHost, FotaPort, "/firmware/file/download", "MODLINK-L/0.0.1");
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
            catM1.ReleaseMutex();
            catHttp.SetSinkToCatFS(false);

            return false;
        }

        catHttp.SetSinkToCatFS(false);
        catM1.ReleaseMutex();
        return true;
    }

    bool PostDownloadResult()
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return false;
        }

        http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        http::RequestHeader header(rest_method_e::POST, http_scheme_e::HTTP, FotaHost, FotaPort, "/firmware/file/download/finish", "MODLINK-L/0.0.1");
        http::RequestBody body("application/x-www-form-urlencoded");
    
        body.AddProperty("mac", MacAddress::GetEthernet());
        body.AddProperty("otaId", std::to_string(info.OtaID));
        body.AddProperty("mcu1.vc", std::to_string(info.mcu1.VersionCode));
        body.AddProperty("mcu1.version", info.mcu1.FirmwareVersion);
        body.AddProperty("mcu1.fileNo", std::to_string(info.mcu1.FileNumberVector.at(0)));
        body.AddProperty("mcu1.filepath", info.mcu1.FilePathVector.at(0));
        body.AddProperty("mcu1.result", "success");

        Status ret = catHttp.POST(mutexHandle.second, header, body);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
            catM1.ReleaseMutex();

           return false;
        }

        return true;
    }
}