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
#include "Common/Convert/ConvertClass.h"
#include "DataFormat/CSV/CSV.h"
#include "IM/Custom/Constants.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    Status FindChunkInfoService(const ota::mcu_e mcuType, const uint8_t idx, ota_chunk_info_t* info)
    {
        ASSERT((info != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        CSV csv;
        Status ret(Status::Code::BAD_NOT_FOUND);

        File file = mcuType == ota::mcu_e::MCU1 ?
            esp32FS.Open(OTA_CHUNK_PATH_ESP32, "r", false) :
            esp32FS.Open(OTA_CHUNK_PATH_MEGA,  "r", false);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA CHUNK INFO FILE");
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }

        int left = 0;
        int right = file.size() - 1;
        int middle = (left + right) / 2;

        while (left <= right)
        {
            middle = (left + right) / 2;
            LOG_DEBUG(logger, "left: %d, middle: %d, right: %d", left, middle, right);

            const bool doesExist = file.seek(middle);
            if (doesExist == false)
            {
                LOG_ERROR(logger, "FAILED TO MOVE FILE POINTER TO '%u'", middle);
                ret = Status::Code::BAD_END_OF_STREAM;
                goto ON_EXIT;
            }

            while ((file.position() > 0) && (file.peek() != '\n'))
            {
                file.seek(file.position() - 1);
            }
            
            if (file.position() == 0)
            {
                break;
            }
            else
            {
                file.read();
            }

            const uint8_t length = 128;
            char line[length] = {'\0'};
            file.readBytesUntil('\n', line, length);
            ret = csv.Decode(line, info);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DECODE LINE: %s", line);
                ret = Status::Code::BAD_END_OF_STREAM;
                goto ON_EXIT;
            }
            LOG_DEBUG(logger, "Decoded: %s", line);

            if (info->Index == idx)
            {
                ret = Status::Code::GOOD;
                LOG_DEBUG(logger, "Found index matched: %u", idx);
                goto ON_EXIT;
            }
            else if (info->Index < idx)
            {
                left = (left + middle) / 2;
            }
            else
            {
                right = (right + middle) / 2;
            }
        }
    
    ON_EXIT:
        file.close();
        return ret;
    }
}