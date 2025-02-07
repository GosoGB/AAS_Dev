/**
 * @file DownloadFirmwareService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어를 다운로드 하는 서비스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <string>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Core/MemoryPool/MemoryPool.h"
#include "DataFormat/CSV/CSV.h"
#include "DownloadFirmwareService.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Protocol/HTTP/IHTTP.h"
#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"

TaskHandle_t sHandle = NULL;



namespace muffin {

    typedef struct DownloadTaskParameterType
    {
        ota::fw_info_t* Info;
        CRC32* _CRC32;
        QueueHandle_t Queue;
        MemoryPool* _MemoryPool;
        void (*Callback)(Status status);
    } download_task_params;

    download_task_params* sParams = nullptr;

    void implementService(void* pvParameters)
    {
        ASSERT((pvParameters != nullptr), "INPUT PARAMETERS CANNOT BE NULL");
        download_task_params* sParams = static_cast<download_task_params*>(pvParameters);

        Status ret(Status::Code::UNCERTAIN);
        while (sParams->Info->Chunk.DownloadIDX < sParams->Info->Chunk.FinishIDX)
        {
            LOG_DEBUG(logger, "Download IDX: %u", sParams->Info->Chunk.DownloadIDX);

            ota_chunk_info_t chunk;
            ret = FindChunkInfoService(sParams->Info->Head.MCU, sParams->Info->Chunk.DownloadIDX, &chunk);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FIND CHUNK WITH GIVEN INDEX: %u", sParams->Info->Chunk.DownloadIDX);
                goto TEARDOWN;
            }

            uint8_t* output = (uint8_t*)sParams->_MemoryPool->Allocate(chunk.Size);
            if (output == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FROM THE POOL. WILL TRY IN SHORTLY");
                ret = Status::Code::BAD_OUT_OF_MEMORY;
                goto TEARDOWN;
            }
            memset(output, 0, chunk.Size);

        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MODLINK_T2)
            char userAgent[32] = "MODLINK-T2/";
        #endif

            http::RequestHeader header(
                rest_method_e::GET,
                sParams->Info->Head.API.Scheme,
                sParams->Info->Head.API.Host,
                sParams->Info->Head.API.Port,
                "/firmware/file/download",
                strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
            );

            ASSERT((sParams->Info->Head.MCU == ota::mcu_e::MCU1 || sParams->Info->Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
            const char* attributeVersionCode      = sParams->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.vc"        : "mcu2.vc";
            const char* attributeSemanticVersion  = sParams->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.version"   : "mcu2.version";
            const char* attributeFileNumber       = sParams->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.fileNo"    : "mcu2.fileNo";
            const char* attributeFilePath         = sParams->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.filepath"  : "mcu2.filepath";

            http::RequestParameter parameters;
            parameters.Add("mac", macAddress.GetEthernet());
            parameters.Add("otaId", std::to_string(sParams->Info->Head.ID));
            parameters.Add(attributeVersionCode, std::to_string(sParams->Info->Head.VersionCode));
            parameters.Add(attributeSemanticVersion, sParams->Info->Head.SemanticVersion);
            parameters.Replace(attributeFileNumber, std::to_string(chunk.Index));
            parameters.Replace(attributeFilePath, chunk.Path);

            INetwork* snic = RetrieveServiceNicService();
            const std::pair<Status, size_t> mutex = snic->TakeMutex();
            if (mutex.first.ToCode() != Status::Code::GOOD)
            {
                sParams->_MemoryPool->Deallocate((void*)output, chunk.Size);
                LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
                ret = mutex.first;
                goto TEARDOWN;
            }

            LOG_DEBUG(logger, "------- 1 -------");
            vTaskDelay(50 / portTICK_PERIOD_MS);
            ret = httpClient->GET(mutex.second, header, parameters, 60);
            LOG_DEBUG(logger, "------- 2 -------");
            if (ret != Status::Code::GOOD)
            {
                sParams->_MemoryPool->Deallocate((void*)output, chunk.Size);
                LOG_ERROR(logger, "FAILED TO DOWNLOAD: %s", ret.c_str());
                snic->ReleaseMutex();
                goto TEARDOWN;
            }

            LOG_DEBUG(logger, "------- 3 -------");
            vTaskDelay(50 / portTICK_PERIOD_MS);
            ret = httpClient->Retrieve(mutex.second, chunk.Size, output);
            LOG_DEBUG(logger, "------- 4 -------");
            if (ret != Status::Code::GOOD)
            {
                sParams->_MemoryPool->Deallocate((void*)output, chunk.Size);
                LOG_ERROR(logger, "FAILED TO RETRIEVE: %s", ret.c_str());
                snic->ReleaseMutex();
                goto TEARDOWN;
            }
            snic->ReleaseMutex();

            const uint32_t integerCRC32 = sParams->_CRC32->Calculate(chunk.Size, output);
            char calculatedCRC32[sizeof(ota::fw_info_t::TotalChecksum)] = {'\0'};
            snprintf(calculatedCRC32, sizeof(ota::fw_info_t::TotalChecksum), "%08x", integerCRC32);
            if (strcmp(calculatedCRC32, chunk.CRC32) != 0)
            {
                sParams->_MemoryPool->Deallocate((void*)output, chunk.Size);
                LOG_ERROR(logger, "INVALID CRC32: %s != %s", calculatedCRC32, chunk.CRC32);
                ret = Status::Code::BAD_DATA_LOST;
                goto TEARDOWN;
            }
            LOG_DEBUG(logger, "Downloaded: %s", calculatedCRC32);
            {
                Status postResult = PostDownloadResult(*sParams->Info, chunk.Size, chunk.Path, "success");
                LOG_INFO(logger, "[POST] download result api: %s", postResult.c_str());
            }

            ASSERT((sParams->Queue != NULL), "INPUT PARAMETERS CANNOT BE NULL");
            xQueueSend(sParams->Queue, (void*)&output, UINT32_MAX);
            ++sParams->Info->Chunk.DownloadIDX;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        LOG_INFO(logger, "Download finished");
        ret = Status::Code::GOOD;
    
    TEARDOWN:
        sParams->Callback(ret);
        StopDownloadFirmwareService();
    }

    Status DownloadFirmwareService(ota::fw_info_t& info, CRC32& crc32, QueueHandle_t queue, MemoryPool* memoryPool, void (*callback)(Status status))
    {
        if (sHandle != NULL)
        {
            return Status(Status::Code::GOOD);
        }

        ASSERT((httpClient != nullptr), "HTTP CLIENT CANNOT BE NULL");

        download_task_params* pvParameters = (download_task_params*)malloc(sizeof(download_task_params));
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        pvParameters->Info         = &info;
        pvParameters->_CRC32       = &crc32;
        pvParameters->Queue        = queue;
        pvParameters->_MemoryPool  = memoryPool;
        pvParameters->Callback     = callback;

        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implementService,     // Function to be run inside of the task
            "DownloadFirmware",   // The identifier of this task for men
            10*KILLOBYTE,          // Stack memory size to allocate
            pvParameters,	      // Task parameters to be passed to the function
            0,				      // Task Priority for scheduling
            &sHandle,             // The identifier of this task for machines
            1				      // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "Download task has been started");
            return Status(Status::Code::GOOD);

        case pdFAIL:
            LOG_ERROR(logger, "FAILED WITHOUT SPECIFIC REASON");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %u", taskCreationResult);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    void StopDownloadFirmwareService()
    {
        if (sHandle != NULL)
        {
            TaskHandle_t tmp = sHandle;
            sHandle = NULL;

            free(sParams);

            INetwork* snic = RetrieveServiceNicService();
            snic->ReleaseMutex();

            vTaskDelete(tmp);
        }
    }
}