/**
 * @file SendMessageService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 상태를 발행하는 서비스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/Custom/Constants.h"
#include "OTA/Include/TypeDefinitions.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/HTTP/IHTTP.h"
#include "SendMessageService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"



namespace muffin {

    Status PublishFirmwareStatusMessageService()
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
        LOG_INFO(logger, "\n Topic: fota/status \n Payload: %s", buffer);

        mqtt::Message message(mqtt::topic_e::FOTA_STATUS, buffer);
        Status ret = mqtt::cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
            /**
             * @todo Store 실패시 flash 메모리에 저장하는 것과 같은 방법을 
             *       적용하여 실패에 강건하도록 코드를 작성해야 합니다.
             */
            LOG_ERROR(logger, "FAIL TO STORE: %s", ret.c_str());
        }
        
        return ret;
    }
    
    Status implementPostDownloadResult(const ota::fw_info_t& info, const size_t size, const char* path, const char* result)
    {
        INetwork* snic = RetrieveServiceNicService();
        const std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return mutex.first;
        }

        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MODLINK_T2)
            char userAgent[32] = "MODLINK-T2/";
        #endif

        http::RequestHeader header(
            rest_method_e::POST,
            info.Head.API.Scheme,
            info.Head.API.Host,
            info.Head.API.Port,
            "/firmware/file/download",
            strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
        );

        http::RequestBody body("application/x-www-form-urlencoded");
        body.AddProperty("mac", macAddress.GetEthernet());
        body.AddProperty("otaId", std::to_string(info.Head.ID));
        ASSERT((info.Head.MCU == ota::mcu_e::MCU1 || info.Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
        if (info.Head.MCU == ota::mcu_e::MCU1)
        {
            body.AddProperty("mcu1.vc",        std::to_string(info.Head.VersionCode));
            body.AddProperty("mcu1.version",   info.Head.SemanticVersion);
            body.AddProperty("mcu1.fileNo",    std::to_string(info.Chunk.DownloadIDX));
            body.AddProperty("mcu1.filepath",  path);
            body.AddProperty("mcu1.result",    result);
        }
        else
        {
            body.AddProperty("mcu2.vc",        std::to_string(info.Head.VersionCode));
            body.AddProperty("mcu2.version",   info.Head.SemanticVersion);
            body.AddProperty("mcu2.fileNo",    std::to_string(info.Chunk.DownloadIDX));
            body.AddProperty("mcu2.filepath",  path);
            body.AddProperty("mcu2.result",    result);
        }

        Status ret = httpClient->POST(mutex.second, header, body);
        snic->ReleaseMutex();

        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO POST: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "POST succeded");
        }
        return ret;
    }
    
    Status implementPostUpdateResult(const ota::fw_info_t& info, const char* result)
    {
        INetwork* snic = RetrieveServiceNicService();
        const std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return mutex.first;
        }

        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MODLINK_T2)
            char userAgent[32] = "MODLINK-T2/";
        #endif

        http::RequestHeader header(
            rest_method_e::POST,
            info.Head.API.Scheme,
            info.Head.API.Host,
            info.Head.API.Port,
            "/firmware/file/download/finish",
            strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
        );

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(info.Head.ID));
        ASSERT((info.Head.MCU == ota::mcu_e::MCU1 || info.Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
        if (info.Head.MCU == ota::mcu_e::MCU1)
        {
            parameters.Add("mcu1.vc",        std::to_string(info.Head.VersionCode));
            parameters.Add("mcu1.version",   info.Head.SemanticVersion);
            parameters.Add("mcu1.result",    result);
        }
        else
        {
            parameters.Add("mcu2.vc",        std::to_string(info.Head.VersionCode));
            parameters.Add("mcu2.version",   info.Head.SemanticVersion);
            parameters.Add("mcu2.result",    result);
        }

        Status ret = httpClient->POST(mutex.second, header, parameters);
        snic->ReleaseMutex();

        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO POST: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "POST succeded");
        }
        return ret;
    }

    Status PostDownloadResult(const ota::fw_info_t& info, const size_t size, const char* path, const char* result)
    {
        Status ret(Status::Code::UNCERTAIN);
        uint8_t trialCount = 0;

        for (trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            ret = implementPostDownloadResult(info, size, path, result);
            if (ret == Status::Code::GOOD)
            {
                return Status(Status::Code::GOOD);
            }

            LOG_WARNING(logger, "[#%u] COULDN'T POST DOWNLOAD RESULT: %s", trialCount, ret.c_str());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        LOG_ERROR(logger, "FAILED TO POST DOWNLOAD RESULT: %s", ret.c_str());
        return ret;
    }

    Status PostUpdateResult(const ota::fw_info_t& info, const char* result)
    {
        Status ret(Status::Code::UNCERTAIN);
        uint8_t trialCount = 0;

        for (trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            ret = implementPostUpdateResult(info, result);
            if (ret == Status::Code::GOOD)
            {
                return Status(Status::Code::GOOD);
            }

            LOG_WARNING(logger, "[#%u] COULDN'T POST UPDATE RESULT: %s", trialCount, ret.c_str());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        LOG_ERROR(logger, "FAILED TO POST UPDATE RESULT: %s", ret.c_str());
        return ret;
    }
}