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




#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Common/Assert.h"
#include "Common/CRC32/CRC32.h"
#include "Common/Logger/Logger.h"
#include "Common/DataStructure/bitset.h"
#include "Core/MemoryPool/MemoryPool.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "OTA/StrategyESP32/StrategyESP32.h"
#include "ServiceSets/FirmwareUpdateServiceSet/DownloadFirmwareService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FirmwareUpdateService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/ParseUpdateInfoService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"

static QueueHandle_t sQueueHandle;
static muffin::MemoryPool* sMemoryPool;
static const uint16_t BLOCK_SIZE = 10*muffin::KILLOBYTE;
static uint8_t sTrialCount = 0;
static const uint8_t MAX_QUEUE_LENGTH = 5;
static const size_t  QUEUE_ITEM_SIZE  = sizeof(uint8_t*);
    
size_t muffin::ota::fw_head_t::ID = 0;
muffin::ota::url_t muffin::ota::fw_head_t::API;


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
        if (jvs::config::opeartion.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            CatM1& catM1 = CatM1::GetInstance();
            catM1.KillUrcTask(true);
        }
        
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

        sMemoryPool = new(std::nothrow) MemoryPool(BLOCK_SIZE, MAX_QUEUE_LENGTH);
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
            LOG_ERROR(logger, "INVALID TOTAL CRC32: %s != %s", calculatedCRC32, info.Checksum.Total);
            return Status(Status::Code::BAD_DATA_LOST);
        }

        crc32.Reset();
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

    Status strategyESP32(ota::fw_info_t& info, CRC32& crc32)
    {
        ota::StrategyESP32 strategy;
        Status ret = strategy.Init(info);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INIT FOR UPDATE");
            return ret;
        }
        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_STARTED));
        LOG_INFO(logger, "Flashing started successfully");
        
        while (info.Chunk.FlashingIDX < info.Chunk.Count)
        {
            if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                ret = DownloadFirmwareService(info, crc32, sQueueHandle, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED)) == true)
            {
                ret = PostDownloadResult(info, "failure");
                return ret;
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == true)
            {
                ret = validateTotalCRC32(info, crc32);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE");
                    return ret;
                }
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(uxQueueMessagesWaiting(sQueueHandle))) == 0 &&
                     sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == false)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            uint8_t* buffer;
            xQueueReceive(sQueueHandle, &buffer, UINT32_MAX);
            ret = strategy.Write(info.Size.Array[info.Chunk.FlashingIDX], buffer);
            if (ret != Status::Code::GOOD)
            {
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FAILED));
                LOG_ERROR(logger, "FAILED TO FLASH CHUNK");
                return ret;
            }
            
            sMemoryPool->Deallocate(buffer, 1);
        }

        ret = strategy.TearDown();
        if (ret != Status::Code::GOOD)
        {
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FAILED));
            LOG_ERROR(logger, "FAILED TO UPDATE ESP32");
            PostUpdateResult(info, "failure");
            return ret;
        }
        
        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FINISHED));
        LOG_INFO(logger, "Updated ESP32 successfully");
        PostUpdateResult(info, "success");
        return ret;
    }

    Status strategyMEGA2560(ota::fw_info_t& info, CRC32& crc32)
    {
    /*
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
            // @brief 워드 주소 체계에 맞추기 위해 페이지 사이즈를 2로 나눕니다.
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
    */
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
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
            ret = Status::Code::BAD_OUT_OF_MEMORY;
            return ret;
        }
    
        ret = ParseUpdateInfoService(payload, esp32, mega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE INFO");
            return ret;
        }
        LOG_INFO(logger, "Parsed firmware info successfully");

        if ((esp32->Head.HasNewFirmware == false) && (mega2560->Head.HasNewFirmware == false))
        {
            LOG_INFO(logger, "No firmware to update");
            ret = Status::Code::GOOD;
            return ret;
        }
/**
 * @todo 호출자 내부에서 다른 모든 태스크를 죽여놓은 상태여야 함
 * @example StopAllTask();
 */

        CRC32 crc32;
        crc32.Init();

        if (mega2560->Head.HasNewFirmware == true)
        {
            sServiceFlags.reset();

            Status ret = DownloadFirmwareService(*mega2560, crc32, sQueueHandle, downloadTaskCallback);
            if (ret != Status::Code::GOOD)
            {
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                return ret;
            }
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            LOG_INFO(logger, "Start to download firmware for ATmega2560");
            strategyMEGA2560(*mega2560, crc32);
        }
    
        if (esp32->Head.HasNewFirmware == true)
        {
            sServiceFlags.reset();

            Status ret = DownloadFirmwareService(*esp32, crc32, sQueueHandle, downloadTaskCallback);
            if (ret != Status::Code::GOOD)
            {
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                return ret;
            }
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            LOG_INFO(logger, "Start to download firmware for ESP32");
            strategyESP32(*esp32, crc32);
        }

/**
 * @todo 호출자 내부에서 아래 함수 호출 필요함
 * @code {.cxx}
 * #if defined(MODLINK_T2) || defined(MODLINK_B)
 *     spear.Reset();
 * #endif
 *     ESP.restart();
 * @endcode
 */
        ret = Status::Code::GOOD;
        return ret;
    }
}