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
#include "DownloadFirmwareService.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Protocol/HTTP/IHTTP.h"
#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"



namespace muffin {

    typedef struct DownloadTaskParameterType
    {
        ota::fw_info_t* Info;
        CRC32* Crc32;
        QueueHandle_t Queue;
        void (*Callback)(Status status);
    } download_task_params;

   void implementService(void* pvParameters)
    {
        ASSERT((pvParameters != nullptr), "INPUT PARAMETERS CANNOT BE NULL");
        download_task_params* params = static_cast<download_task_params*>(pvParameters);

        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MODLINK_T2)
            char userAgent[32] = "MODLINK-T2/";
        #endif

        http::RequestHeader header(
            rest_method_e::GET,
            params->Info->Head.API.Scheme,
            params->Info->Head.API.Host,
            params->Info->Head.API.Port,
            "/firmware/file/download",
            strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
        );

        ASSERT((params->Info->Head.MCU == ota::mcu_e::MCU1 || params->Info->Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
        const char* attributeVersionCode      = params->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.vc"        : "mcu2.vc";
        const char* attributeSemanticVersion  = params->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.version"   : "mcu2.version";
        const char* attributeFileNumber       = params->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.fileNo"    : "mcu2.fileNo";
        const char* attributeFilePath         = params->Info->Head.MCU == ota::mcu_e::MCU1 ? "mcu1.filepath"  : "mcu2.filepath";

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(params->Info->Head.ID));
        parameters.Add(attributeVersionCode, std::to_string(params->Info->Head.VersionCode));
        parameters.Add(attributeSemanticVersion, params->Info->Head.SemanticVersion);

        INetwork* nic = http::client->RetrieveNIC();
        Status ret(Status::Code::UNCERTAIN);

        while (params->Info->Chunk.DownloadIDX < params->Info->Chunk.Count)
        {
            parameters.Add(attributeFileNumber, std::to_string(params->Info->Chunk.IndexArray[params->Info->Chunk.DownloadIDX]));
            parameters.Add(attributeFilePath, params->Info->Chunk.PathArray[params->Info->Chunk.DownloadIDX]);

            const std::pair<Status, size_t> mutex = nic->TakeMutex();
            if (mutex.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
                ret = mutex.first;
                goto TEARDOWN;
            }
            
            ret = http::client->GET(mutex.second, header, parameters, 300);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD: %s", ret.c_str());
                nic->ReleaseMutex();
                goto TEARDOWN;
            }

            std::string* output = new std::string();
            ret = http::client->Retrieve(mutex.second, output);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE: %s", ret.c_str());
                nic->ReleaseMutex();
                goto TEARDOWN;
            }
            else if (output->length() != params->Info->Size.Array[params->Info->Chunk.DownloadIDX])
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE: %s", ret.c_str());
                ret = Status::Code::BAD_DATA_LOST;
                nic->ReleaseMutex();
                goto TEARDOWN;
            }
            nic->ReleaseMutex();

            const uint32_t integerCRC32 = params->Crc32->Calculate(*output);
            char calculatedCRC32[sizeof(ota::fw_cks_t::Array[0])] = {'\0'};
            snprintf(calculatedCRC32, sizeof(ota::fw_cks_t::Array[0]), "%08x", integerCRC32);
            if (strcmp(calculatedCRC32, params->Info->Checksum.Array[params->Info->Chunk.DownloadIDX]) != 0)
            {
                LOG_ERROR(logger, "INVALID CRC32: %s != %s", calculatedCRC32, params->Info->Checksum.Array[params->Info->Chunk.DownloadIDX]);
                ret = Status::Code::BAD_DATA_LOST;
                goto TEARDOWN;
            }
            LOG_DEBUG(logger, "Downloaded: %s", calculatedCRC32);
            {
                Status postResult = PostDownloadResult(*params->Info, "success");
                if (postResult == Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO POST DOWNLOAD RESULT: %s", postResult.c_str());
                }
            }

            ASSERT((params->Queue != NULL), "INPUT PARAMETERS CANNOT BE NULL");
            xQueueSend(params->Queue, output, UINT32_MAX);
            ++params->Info->Chunk.DownloadIDX;
        }
        LOG_INFO(logger, "Download finished");
        ret = Status::Code::GOOD;
    
    TEARDOWN:
        params->Callback(ret);
        vTaskDelete(NULL);
    }

    Status DownloadFirmwareService(ota::fw_info_t& info, CRC32& crc32, QueueHandle_t queue, void (*callback)(Status status))
    {
        ASSERT((http::client != nullptr), "HTTP CLIENT CANNOT BE NULL");
        ASSERT((uxQueueMessagesWaiting(queue) == 0), "QUEUE MUST BE EMPTY");

        download_task_params* pvParameters = (download_task_params*)malloc(sizeof(download_task_params));
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        pvParameters->Info       = &info;
        pvParameters->Crc32      = &crc32;
        pvParameters->Queue      = queue;
        pvParameters->Callback   = callback;

        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implementService,     // Function to be run inside of the task
            "DownloadFirmware",   // The identifier of this task for men
            4 * KILLOBYTE,        // Stack memory size to allocate
            pvParameters,	      // Task parameters to be passed to the function
            0,				      // Task Priority for scheduling
            NULL,                 // The identifier of this task for machines
            0				      // Index of MCU core where the function to run
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
}