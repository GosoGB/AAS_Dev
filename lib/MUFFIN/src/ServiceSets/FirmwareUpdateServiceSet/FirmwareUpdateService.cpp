/**
 * @file FirmwareUpdateService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어를 업데이트하는 서비스를 선언합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




// #include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Common/Assert.h"
#include "Common/CRC32/CRC32.h"
#include "Common/Logger/Logger.h"
#include "Common/DataStructure/bitset.h"
#include "Common/Status.h"
#include "Core/MemoryPool/MemoryPool.h"
#include "IM/Custom/Constants.h"

#include "DownloadFirmwareService.h"
#include "ParseFirmwareUpdateInfoService.h"

static TaskHandle_t sDownloadTaskHandle;
static TaskHandle_t sFlashingTaskHandle;
static QueueHandle_t sQueueHandle;
static muffin::MemoryPool* sMemoryPool;
static uint8_t sTrialCount = 0;
static const uint8_t MAX_QUEUE_LENGTH = 5;
static const size_t  QUEUE_ITEM_SIZE  = sizeof(uint8_t*);

typedef enum class ServiceStatusEnum : uint8_t
{
    DOWNLOAD_STARTED    = 0,
    FLASHING_STARTED    = 1,
    DOWNLOAD_FINISHED   = 2,
    FLASHING_FINISHED   = 3,
    DOWNLOAD_FAILED     = 4,
    FLASHING_FAILED     = 5,
    TRY_DOWNLOAD_AGAIN  = 6,
    TOP                 = 7
} srv_status_e;
muffin::bitset<static_cast<uint8_t>(srv_status_e::TOP)> sServiceFlags;



namespace muffin {

    Status initializeService()
    {
        if (sQueueHandle != NULL && sMemoryPool != nullptr)
        {
            LOG_DEBUG(logger, "NOTHING TO DO: Already initialized");
            return Status(Status::Code::GOOD);
        }
        
        sQueueHandle = xQueueCreate(MAX_QUEUE_LENGTH, QUEUE_ITEM_SIZE);
        if (sQueueHandle == NULL)
        {
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        sMemoryPool = new(std::nothrow) MemoryPool(10*KILLOBYTE, MAX_QUEUE_LENGTH);
        if (sMemoryPool == nullptr)
        {
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        LOG_DEBUG(logger, "Iniialized FirmwareUpdateService");
        return Status(Status::Code::GOOD);
    }

    Status validateTotalCRC32(ota::fw_info_t& info, CRC32& crc32)
    {
        crc32.Teardown();
        const uint32_t integerCRC32 = crc32.RetrieveTotalChecksum();

        char calculatedCRC32[sizeof(ota::fw_cks_t::Array[0])] = {'\0'};
        snprintf(calculatedCRC32, sizeof(ota::fw_cks_t::Array[0]), "%08x", integerCRC32);
        if (strcmp(calculatedCRC32, info.Checksum.Total) != 0)
        {
            LOG_ERROR(logger, "INVALID CRC32: %s != %s", calculatedCRC32, info.Checksum.Total);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        return Status(Status::Code::GOOD);
    }

    void downloadTaskCallback(Status status)
    {
        if (status != Status::Code::GOOD)
        {
            if (sTrialCount == MAX_RETRY_COUNT)
            {
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
            }
            else
            {
                ++sTrialCount;
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
        }
        else
        {
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED));
            sTrialCount = 0;
        }
    }

    Status strategyESP32()
    {
        ;
    }

    Status strategyMEGA2560(ota::fw_info_t& info, CRC32& crc32)
    {
        Status ret = DownloadFirmwareService(info, crc32, sQueueHandle, downloadTaskCallback);
        if (ret != Status::Code::GOOD)
        {
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
            LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
            return ret;
        }
        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
        LOG_INFO(logger, "Start to download firmware for ATmega2560");
        
        while (true)
        {
            if (static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN) == true)
            {
                Status ret = DownloadFirmwareService(info, crc32, sQueueHandle, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            
            if (static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED) == true)
            {
                ret = Status::Code::BAD;
                return ret;
            }
            
            if (static_cast<uint8_t>(uxQueueMessagesWaiting(sQueueHandle)) == 0 &&
                static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED) == false)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            // mega2560 업데이트 시작
                // 펌웨어 다운로드 태스크 시작
                    // 태스크 간 chunk 전달
                // 펌웨어 업데이트 태스크 시작
        }

        ret = validateTotalCRC32(info, crc32);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE ATmega2560");
        }
        LOG_INFO(logger, "ATmega2560 updated successfully");
    }

    Status FirmwareUpdateService(const char* payload)
    {
        Status ret = initializeService();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE: %s", ret.c_str());
            return ret;
        }        

        ota::fw_info_t* esp32 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));
        ota::fw_info_t* mega2560 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));
        if (esp32 == nullptr || mega2560 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR FW INFO");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
    
        ret = ParseFirmwareUpdateInfoService(payload, esp32, mega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE UPDATE INFO");
            return ret;
        }
        LOG_INFO(logger, "Parsed successfully");

        CRC32 crc32;
        crc32.Init();

        if (mega2560->Head.HasNewFirmware == true)
        {
            strategyMEGA2560(*mega2560, crc32);
        }
    
        if (esp32->Head.HasNewFirmware == true)
        {
            // esp32 업데이트 시작
                // 펌웨어 다운로드 태스크 시작
                    // 태스크 간 chunk 전달
                // 펌웨어 업데이트 태스크 시작

            ret = validateTotalCRC32(*esp32, crc32);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE ESP32");
                ret = Status::Code::BAD_DATA_LOST;
                return ret;
            }
            LOG_INFO(logger, "ESP32 updated successfully");
        }

        ret = Status::Code::GOOD;
        return ret;
    }
}