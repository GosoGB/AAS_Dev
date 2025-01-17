/**
 * @file FirmwareUpdateService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어를 업데이트하는 서비스를 선언합니다.
 * 
 * @date 2025-01-16
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




// #include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Common/Assert.h"
#include "Common/CRC32/CRC32.h"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "Core/MemoryPool/MemoryPool.h"
#include "IM/Custom/Constants.h"

#include "DownloadFirmwareService.h"
#include "ParseFirmwareUpdateInfoService.h"

static QueueHandle_t sQueueHandle;
static muffin::MemoryPool* sMemoryPool;



namespace muffin {

    Status initializeService()
    {
        if (sQueueHandle != NULL && sMemoryPool != nullptr)
        {
            LOG_DEBUG(logger, "NOTHING TO DO: Already initialized");
            return Status(Status::Code::GOOD);
        }

        const uint8_t MAX_QUEUE_LENGTH = 5;
        const size_t  QUEUE_ITEM_SIZE  = sizeof(uint8_t*);
        
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
        const uint32_t integerCRC32 = crc32.RetrieveTotalChecksum();
        crc32.Teardown();

        char stringCRC32[sizeof(ota::fw_cks_t::ChunkArray[0])] = {'\0'};
        snprintf(stringCRC32, sizeof(ota::fw_cks_t::ChunkArray[0]), "%08x", integerCRC32);
        if (strcmp(stringCRC32, info.Checksum.ChunkArray[info.Chunk.Index]) != 0)
        {
            LOG_ERROR(logger, "INVALID CRC32: %s != %s", stringCRC32, info.Checksum.ChunkArray[info.Chunk.Index]);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        return Status(Status::Code::GOOD);
    }

    Status strategyESP32()
    {
        ;
    }

    Status strategyMEGA2560()
    {
        ;
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
            // mega2560 업데이트 시작
                // 펌웨어 다운로드 태스크 시작
                    // 태스크 간 chunk 전달
                // 펌웨어 업데이트 태스크 시작

            ret = validateTotalCRC32(*mega2560, crc32);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE ATmega2560");
            }
            LOG_INFO(logger, "ATmega2560 updated successfully");
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