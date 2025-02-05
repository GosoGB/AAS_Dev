/**
 * @file FindChunkInfoService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPIFFS 파티션으로부터 특정 OTA 청크 파일에 대한 정보를 찾아 반환하는 서비스를 정의합니다.
 * 
 * @date 2025-02-05
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "DataFormat/CSV/CSV.h"
#include "IM/Custom/Constants.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    Status FindChunkInfoService(const uint8_t idx, ota_chunk_info_t* info)
    {
        ASSERT((info != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        File file = esp32FS.Open(OTA_CHUNK_INFO_PATH, "r", false);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA CHUNK INFO FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        const size_t filePointer = 0; 여기 로직 맹글어야 함
        const bool isSought = file.seek(filePointer);
        if (isSought == false)
        {
            LOG_ERROR(logger, "FAILED TO MOVE FILE POINTER TO '%u'", filePointer);
            return Status(Status::Code::BAD_END_OF_STREAM);
        }
        
        Status ret(Status::Code::UNCERTAIN);
        const uint8_t length = 128;
        char line[length] = {'\0'};

        for (uint8_t idx = 0; idx < length; ++idx)
        {
            const int value = file.read();

            switch (value)
            {
            case -1:
                LOG_ERROR(logger, "FAILED TO READ DATA FROM FLASH MEMORY");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                goto END_OF_READ;

            case '\r':
                line[idx] = '\0';
                break;

            case '\n':
                LOG_DEBUG(logger, "Line: %s", line);
                ret = Status::Code::GOOD;
                goto END_OF_READ;

            default:
                line[idx] = value;
                break;
            }
        }

    END_OF_READ:
        file.close();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        memset(info, 0, sizeof(ota_chunk_info_t));

        CSV csv;
        ret = csv.Decode(line, info);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DECODE LINE: %s", line);
        }
        
        return ret;
    }
}