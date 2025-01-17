/**
 * @file DownloadFirmwareService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어를 다운로드 하는 서비스를 정의합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <string>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "DownloadFirmwareService.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Protocol/HTTP/IHTTP.h"



namespace muffin {

    Status implementService(ota::fw_info_t& info, CRC32& crc32, QueueHandle_t queue)
    {

        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MODLINK_T2)
            char userAgent[32] = "MODLINK-T2/";
        #endif

        http::RequestHeader header(
            rest_method_e::GET,
            info.Head.DownloadURL.Scheme,
            info.Head.DownloadURL.Host,
            info.Head.DownloadURL.Port,
            "/firmware/file/download",
            strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
        );

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(info.Head.ID));
        ASSERT((info.Head.MCU == ota::mcu_e::MCU1 || info.Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
        const char* attributeVersionCode      = info.Head.MCU == ota::mcu_e::MCU1 ? "mcu1.vc"        : "mcu2.vc";
        const char* attributeSemanticVersion  = info.Head.MCU == ota::mcu_e::MCU1 ? "mcu1.version"   : "mcu2.version";
        const char* attributeFileNumber       = info.Head.MCU == ota::mcu_e::MCU1 ? "mcu1.fileNo"    : "mcu2.fileNo";
        const char* attributeFilePath         = info.Head.MCU == ota::mcu_e::MCU1 ? "mcu1.filepath"  : "mcu2.filepath";
        
        while (info.Chunk.Index < info.Chunk.Count)
        {
            parameters.Add(attributeVersionCode,       std::to_string(info.Head.VersionCode));
            parameters.Add(attributeSemanticVersion,   info.Head.SemanticVersion);
            parameters.Add(attributeFileNumber,        std::to_string(info.Chunk.IndexArray[info.Chunk.Index]));
            parameters.Add(attributeFilePath,          info.Chunk.PathArray[info.Chunk.Index]);
        }
        vTaskDelete(NULL);

        

    }

    Status DownloadFirmwareService(ota::fw_info_t& info, CRC32& crc32, QueueHandle_t queue)
    {
        ASSERT((http::client != nullptr), "HTTP CLIENT CANNOT BE NULL");
        ASSERT((uxQueueMessagesWaiting(queue) == 0), "QUEUE MUST BE EMPTY");

        INetwork* nic = http::client->RetrieveNIC();
        const std::pair<Status, size_t> mutex = nic->TakeMutex();
        if (mutex.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return mutex.first;
        }
        
        Status ret = http::client->GET(mutex.second, header, parameters, 300);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DOWNLOAD: %s", ret.c_str());
            nic->ReleaseMutex();
            return ret;
        }

        ret = http::client->Retrieve(mutex.second, output);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE: %s", ret.c_str());
            nic->ReleaseMutex();
            return ret;
        }
        nic->ReleaseMutex();

        const uint32_t integerCRC32 = crc32.Calculate(*output);
        char stringCRC32[sizeof(ota::fw_cks_t::ChunkArray[0])] = {'\0'};
        snprintf(stringCRC32, sizeof(ota::fw_cks_t::ChunkArray[0]), "%08x", integerCRC32);
        if (strcmp(stringCRC32, info.Checksum.ChunkArray[info.Chunk.Index]) != 0)
        {
            LOG_ERROR(logger, "INVALID CRC32: %s != %s", stringCRC32, info.Checksum.ChunkArray[info.Chunk.Index]);
            ret = Status::Code::BAD_DATA_LOST;
            return ret;
        }
        LOG_DEBUG(logger, "Downloaded: %s", stringCRC32);
        return ret;
    }
}