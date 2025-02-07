/**
 * @file FirmwareUpdateService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어를 업데이트하는 서비스를 선언합니다.
 * 
 * @date 2025-02-04
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
#include "OTA/MEGA2560/HexParser.h"
#include "OTA/MEGA2560/MEGA2560.h"
#include "OTA/StrategyESP32/StrategyESP32.h"
#include "ServiceSets/FirmwareUpdateServiceSet/DownloadFirmwareService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FetchFirmwareInfo.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FirmwareUpdateService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/ParseUpdateInfoService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"
#include "Storage/ESP32FS/ESP32FS.h"

static QueueHandle_t sQueueHandle;
static TaskHandle_t sParsingTaskHandle;
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
    PARSING_CHUNK_FILE  = 7,
    TOP                 = 8
} srv_status_e;
muffin::bitset<static_cast<uint8_t>(srv_status_e::TOP)> sServiceFlags;



namespace muffin {

    Status initializeService()
    {
        memoryPool.~MemoryPool();

        if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            ASSERT((catM1 != nullptr), "CatM1 INSTANCE CANNOT BE NULL");
            catM1->KillUrcTask(true);
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
        const size_t length = sizeof(ota::fw_info_t::TotalChecksum);
        char calculatedCRC32[length] = {'\0'};

        crc32.Teardown();
        const uint32_t integerCRC32 = crc32.RetrieveTotalChecksum();

        snprintf(calculatedCRC32, sizeof(ota::fw_info_t::TotalChecksum), "%08x", integerCRC32);
        if (strcmp(calculatedCRC32, info.TotalChecksum) != 0)
        {
            LOG_ERROR(logger, "INVALID TOTAL CRC32: %s != %s", calculatedCRC32, info.TotalChecksum);
            return Status(Status::Code::BAD_DATA_LOST);
        }

        crc32.Reset();
        LOG_DEBUG(logger, "Total CRC32: %s", calculatedCRC32);
        return Status(Status::Code::GOOD);
    }

    void downloadTaskCallback(Status status)
    {
        if (status != Status::Code::GOOD)
        {
            LOG_INFO(logger, "Received download task callback: %s", status.c_str());

            if (sTrialCount == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", status.c_str());
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
            }
            else
            {
                ++sTrialCount;
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
                LOG_DEBUG(logger, "Set download task flag: \"TRY_DOWNLOAD_AGAIN\"");
            }
        }
        else
        {
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
            sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED));
            sTrialCount = 0;
            LOG_INFO(logger, "Download task has finished: %s", status.c_str());
        }
    }

    Status strategyESP32(ota::fw_info_t& info, CRC32& crc32)
    {
        while (uxQueueMessagesWaiting(sQueueHandle) < (MAX_QUEUE_LENGTH - 1))
        {
            if ((sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED)) == true))
            {
                return Status(Status::Code::BAD_COMMUNICATION_ERROR);
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                LOG_INFO(logger, "Restart to download firmware");
                Status ret = DownloadFirmwareService(info, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        ota::StrategyESP32 strategy;
        Status ret = strategy.Init(info);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INIT FOR UPDATE");
            return ret;
        }
        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_STARTED));
        LOG_INFO(logger, "Flashing started successfully");
        
        while (info.Chunk.FlashingIDX < info.Chunk.FinishIDX)
        {
            ota_chunk_info_t chunk;
            ret = FindChunkInfoService(info.Head.MCU, info.Chunk.FlashingIDX, &chunk);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FIND CHUNK WITH GIVEN INDEX: %u", info.Chunk.DownloadIDX);
                return ret;
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                LOG_INFO(logger, "Restart to download firmware");
                Status ret = DownloadFirmwareService(info, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            
            if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                LOG_INFO(logger, "Restart to download firmware");
                ret = DownloadFirmwareService(info, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
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
                Status postResult = PostDownloadResult(info, chunk.Size, chunk.Path, "failure");
                LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", ret.c_str());
                ret = Status::Code::BAD_DATA_LOST;
                return ret;
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == true)
            {
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED));
                ret = validateTotalCRC32(info, crc32);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE");
                    return ret;
                }
            }

            while (uxQueueMessagesWaiting(sQueueHandle) > 0)
            {
                uint8_t* buffer;
                xQueueReceive(sQueueHandle, &buffer, static_cast<TickType_t>(SECOND_IN_MILLIS));
                ret = strategy.Write(chunk.Size, buffer);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FAILED));
                    LOG_ERROR(logger, "FAILED TO FLASH CHUNK");
                    return ret;
                }

                sMemoryPool->Deallocate(buffer, chunk.Size);
                ++info.Chunk.FlashingIDX;
            }
        }

        const Status updateResult = strategy.TearDown();
        if (updateResult.ToCode() != Status::Code::GOOD)
        {
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FAILED));
            LOG_ERROR(logger, "FAILED TO UPDATE ESP32");
            Status postResult = PostUpdateResult(info, "failure");
            LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
            return updateResult;
        }
        
        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FINISHED));
        LOG_INFO(logger, "Updated ESP32 successfully");
        Status postResult = PostUpdateResult(info, "success");
        LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
        return ret;
    }

    Status waitTillPageParsed(const uint32_t delayMillis, const uint32_t address, ota::MEGA2560& mega2560)
    {
        const uint32_t startedMillis = millis();
        const uint32_t pageSize = 256;

    LOAD_FIRST_BYTE:
        uint32_t currentAddress = 0;
        Status ret = mega2560.LoadAddress(currentAddress);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO LOAD ADDRESS: %s", ret.c_str());
            return ret;
        }

        while ((millis() - startedMillis) < delayMillis)
        {
            if (currentAddress == address)
            {
                goto LOAD_FIRST_BYTE;
            }
            
            ota::page_t pageReadBack;
            ret = mega2560.ReadFlashISP(pageSize, &pageReadBack);
            currentAddress += pageSize;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        
        ret = mega2560.LoadAddress(address);
        LOG_INFO(logger, "Loading address to %u: %s", address, ret.c_str());
        return ret;
    }

    typedef struct ParsingTaskParameterType
    {
        ota::HexParser* Parser;
        ota::fw_info_t* Info;
    } parsing_task_params;

    void parseChunkPage(void* pvParameters)
    {
        ASSERT((pvParameters != nullptr), "INPUT PARAMETERS CANNOT BE NULL");
        parsing_task_params* params = static_cast<parsing_task_params*>(pvParameters);

        while (true)
        {
            if (uxQueueMessagesWaiting(sQueueHandle) == 0)
            {
                if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == true)
                {
                    goto ON_EXIT;
                }
                
                vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
                continue;
            }
            
            ota_chunk_info_t chunkInfo;
            Status ret = FindChunkInfoService(params->Info->Head.MCU, params->Info->Chunk.FlashingIDX, &chunkInfo);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FIND CHUNK WITH GIVEN INDEX: %u", params->Info->Chunk.FlashingIDX);
                break;
            }

            uint8_t* buffer;
            xQueueReceive(sQueueHandle, &buffer, static_cast<TickType_t>(SECOND_IN_MILLIS));
            std::string chunk(reinterpret_cast<char*>(buffer), chunkInfo.Size);
            sMemoryPool->Deallocate(buffer, chunkInfo.Size);

            ret = params->Parser->Parse(chunk);
            if ((ret != Status::Code::GOOD) && (ret != Status::Code::GOOD_MORE_DATA))
            {
                LOG_ERROR(logger, "FAILED TO PARSE CHUNK: %s", ret.c_str());
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                break;
            }
            else
            {
                ++params->Info->Chunk.FlashingIDX;
                LOG_DEBUG(logger, "FlashingIDX: %u", params->Info->Chunk.FlashingIDX);
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }
    
    ON_EXIT:
        free(params);
        sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::PARSING_CHUNK_FILE));
        vTaskDelete(sParsingTaskHandle);
        LOG_WARNING(logger, "PAGE PARSING TASK HAS BEEN TERMINATED");
    }

    Status startChunkPageParsing(ota::fw_info_t& info, ota::HexParser& parser)
    {
        if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::PARSING_CHUNK_FILE)) == true)
        {
            return Status(Status::Code::GOOD);
        }
        
        parsing_task_params* pvParameters = (parsing_task_params*)malloc(sizeof(parsing_task_params));
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        pvParameters->Info    = &info;
        pvParameters->Parser  = &parser;

        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            parseChunkPage,       // Function to be run inside of the task
            "parseChunkPage",     // The identifier of this task for men
            3*KILLOBYTE,          // Stack memory size to allocate
            pvParameters,	      // Task parameters to be passed to the function
            0,				      // Task Priority for scheduling
            &sParsingTaskHandle,  // The identifier of this task for machines
            0				      // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "Parsing chunk task has been started");
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::PARSING_CHUNK_FILE));
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

    Status strategyMEGA2560(ota::fw_info_t& info, CRC32& crc32)
    {
        ota::MEGA2560 mega2560;
        ota::HexParser hexParser;
        size_t currentAddress = 0;

        while (uxQueueMessagesWaiting(sQueueHandle) < (MAX_QUEUE_LENGTH - 1))
        {
            if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED)) == true)
            {
                return Status(Status::Code::BAD_COMMUNICATION_ERROR);
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                LOG_INFO(logger, "Restart to download firmware");
                Status ret = DownloadFirmwareService(info, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        startChunkPageParsing(info, hexParser);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        Status ret = mega2560.Init(info.TotalSize);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE: FAILED TO INIT UPDATE FOR ATmega2560: %s", ret.c_str());

            Status postResult = PostUpdateResult(info, "failure");
            LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());

            mega2560.TearDown();

            StopDownloadFirmwareService();
            vTaskDelete(sParsingTaskHandle);
            sParsingTaskHandle = NULL;
            return ret;
        }

        while (info.Chunk.FlashingIDX < info.Chunk.FinishIDX)
        {
            LOG_DEBUG(logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());

            ota_chunk_info_t chunk;
            ret = FindChunkInfoService(info.Head.MCU, info.Chunk.FlashingIDX, &chunk);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FIND CHUNK WITH GIVEN INDEX: %u", info.Chunk.FlashingIDX);
                mega2560.TearDown();

                vTaskDelete(sParsingTaskHandle);
                sParsingTaskHandle = NULL;
                return ret;
            }
            
            if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN)) == true)
            {
                LOG_INFO(logger, "Restart to download firmware");
                ret = DownloadFirmwareService(info, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
                if (ret != Status::Code::GOOD)
                {
                    sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                    LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                    mega2560.TearDown();

                    vTaskDelete(sParsingTaskHandle);
                    sParsingTaskHandle = NULL;
                    return ret;
                }
                sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::TRY_DOWNLOAD_AGAIN));
            }
            else if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED)) == true)
            {
                LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE: %s", ret.c_str());
                mega2560.TearDown();
                
                Status postResult = PostDownloadResult(info, chunk.Size, chunk.Path, "failure");
                LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());

                vTaskDelete(sParsingTaskHandle);
                sParsingTaskHandle = NULL;

                ret = Status::Code::BAD_DATA_LOST;
                return ret;
            }

            while (hexParser.GetPageCount() > 0)
            {
                ota::page_t page = hexParser.GetPage();

                for (uint8_t trial = 0; trial < MAX_RETRY_COUNT; ++trial)
                {
                    ret = mega2560.LoadAddress(currentAddress);
                    if (ret == Status::Code::GOOD)
                    {
                        break;
                    }
                }

                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO LOAD ADDRESS: %u", currentAddress);
                    Status postResult = PostUpdateResult(info, "failure");
                    LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
                    mega2560.TearDown();
                    return Status(Status::Code::BAD_COMMUNICATION_ERROR);
                }
                
                for (uint8_t trial = 0; trial < MAX_RETRY_COUNT; ++trial)
                {
                    ret = mega2560.ProgramFlashISP(page);
                    if (ret == Status::Code::GOOD)
                    {
                        break;
                    }
                }
                            
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO FLASH DATA: %u", currentAddress);
                    Status postResult = PostUpdateResult(info, "failure");
                    LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
                    mega2560.TearDown();
                    return Status(Status::Code::BAD_COMMUNICATION_ERROR);
                }

                ota::page_t pageReadBack;
                for (size_t i = 0; i < 5; ++i)
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
                    LOG_ERROR(logger, "FAILED TO UPDATE: READ BACK PAGE SIZE DOES NOT MATCH: Target: %u, Actual: %u",
                        page.Size, pageReadBack.Size);
                        
                    mega2560.TearDown();
                    Status postResult = PostUpdateResult(info, "failure");
                    LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
                    return Status(Status::Code::BAD_DATA_LOST);
                }

                for (size_t i = 0; i < page.Size; ++i)
                {
                    if (page.Data[i] != pageReadBack.Data[i])
                    {
                        LOG_ERROR(logger, "FAILED TO UPDATE: WRITTEN DATA DOES NOT MATCH: Target: %u, Actual: %u",
                            page.Data[i], pageReadBack.Data[i]);
                        
                        mega2560.TearDown();
                        Status postResult = PostUpdateResult(info, "failure");
                        LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
                        return Status(Status::Code::BAD_DATA_LOST);
                    }
                }

                hexParser.RemovePage();
                vTaskDelay(10 / portTICK_PERIOD_MS);
                LOG_DEBUG(muffin::logger, "Page Remained: %u", hexParser.GetPageCount());

                /**
                 * @brief 워드 주소 체계에 맞추기 위해 페이지 사이즈를 2로 나눕니다.
                 * @details MODLINK-T2 내부의 ATmega2560 칩셋은 부트로더 내부에 AVRISP_2 프로그래머를 가지고 있습니다.
                 *          AVRISP_2 워드 주소 체계는 16-bit 단위이기 때문에 두 개의 바이트가 하나의 주소로 표현됩니다.
                 */
                currentAddress += page.Size / 2;

                if (sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == true)
                {
                    sServiceFlags.reset(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED));
                    ret = validateTotalCRC32(info, crc32);
                    if (ret != Status::Code::GOOD)
                    {
                        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                        LOG_ERROR(logger, "FAILED TO DOWNLOAD FIRMWARE");
                        mega2560.TearDown();

                        vTaskDelete(sParsingTaskHandle);
                        sParsingTaskHandle = NULL;
                        return ret;
                    }
                }

                if ((sServiceFlags.test(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FINISHED)) == false) &&
                    (hexParser.GetPageCount() == 0))
                {
                    waitTillPageParsed(5*SECOND_IN_MILLIS, currentAddress, mega2560);
                    break;
                }
            }
            waitTillPageParsed(SECOND_IN_MILLIS, currentAddress, mega2560);
        }

        sServiceFlags.set(static_cast<uint8_t>(srv_status_e::FLASHING_FINISHED));
        mega2560.LeaveProgrammingMode();
        mega2560.TearDown();
        
        LOG_INFO(logger, "Updated ATmega2560 successfully");
        Status postResult = PostUpdateResult(info, "success");

        LOG_INFO(logger, "[POST] update result api: %s", postResult.c_str());
        return Status(Status::Code::GOOD);
    }

    Status FirmwareUpdateService()
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

        {
            File file = esp32FS.Open(OTA_REQUEST_PATH, "r", false);
            if (file == false)
            {
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }

            const size_t size = file.size();
            char buffer[size] = {'\0'};
            file.readBytes(buffer, size);
            file.close();
            
            ret = ParseUpdateInfoService(buffer, esp32, mega2560);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE INFO");
                return ret;
            }
            LOG_INFO(logger, "Parsed firmware info successfully");
        }

        {    
            std::string payload;
            ret = FetchFirmwareInfo(*esp32, &payload);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FETCH FIRMWARE INFO");
                return ret;
            }

            ret = ParseUpdateInfoService(payload.c_str(), esp32, mega2560);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE INFO");
                return ret;
            }
            payload.clear();
            payload.shrink_to_fit();
            LOG_INFO(logger, "Parsed firmware info successfully");
        }

        if ((esp32->Head.HasNewFirmware == false) && (mega2560->Head.HasNewFirmware == false))
        {
            LOG_INFO(logger, "No firmware to update");
            ret = Status::Code::GOOD;
            return ret;
        }

        CRC32 crc32;
        crc32.Init();

        if (mega2560->Head.HasNewFirmware == true)
        {
            sServiceFlags.reset();

            Status ret = DownloadFirmwareService(*mega2560, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
            if (ret != Status::Code::GOOD)
            {
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                return ret;
            }
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            LOG_INFO(logger, "Start to download firmware for ATmega2560");
            ret = strategyMEGA2560(*mega2560, crc32);
            LOG_INFO(logger, "Update Result for ATmega2560: %s", ret.c_str());
            if (ret != Status::Code::GOOD)
            {
                StopDownloadFirmwareService();
            }
        }

        sMemoryPool->Reset();
    
        if (esp32->Head.HasNewFirmware == true)
        {
            sServiceFlags.reset();

            Status ret = DownloadFirmwareService(*esp32, crc32, sQueueHandle, sMemoryPool, downloadTaskCallback);
            if (ret != Status::Code::GOOD)
            {
                sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_FAILED));
                LOG_ERROR(logger, "FAILED TO START DOWNLOAD TASK");
                return ret;
            }
            sServiceFlags.set(static_cast<uint8_t>(srv_status_e::DOWNLOAD_STARTED));
            LOG_INFO(logger, "Start to download firmware for ESP32");
            ret = strategyESP32(*esp32, crc32);
            LOG_INFO(logger, "Update Result for ESP32: %s", ret.c_str());
            if (ret != Status::Code::GOOD)
            {
                StopDownloadFirmwareService();
                sMemoryPool->Reset();
            }
        }

        return ret;
    }
}