/**
 * @file FindChunkInfoService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPIFFS 파티션으로부터 특정 OTA 청크 파일에 대한 정보를 찾아 반환하는 서비스를 정의합니다.
 * 
 * @date 2025-02-06
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "DataFormat/CSV/CSV.h"
#include "IM/Custom/Constants.h"
#include "ServiceSets/FirmwareUpdateServiceSet/FindChunkInfoService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    Status ReadIndexFromFirstLine(const ota::mcu_e mcuType, uint8_t* index)
    {
        ASSERT((index != nullptr), "INPUT PARAMETER CANNOT BE NULL");

        Status ret = mcuType == ota::mcu_e::MCU1 ? esp32FS.DoesExist(OTA_CHUNK_PATH_ESP32)
                                                 : esp32FS.DoesExist(OTA_CHUNK_PATH_MEGA);
        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            return ret;
        }

        File file = mcuType == ota::mcu_e::MCU1 ? esp32FS.Open(OTA_CHUNK_PATH_ESP32, "r", false)
                                                : esp32FS.Open(OTA_CHUNK_PATH_MEGA,  "r", false);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA CHUNK INFO FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        const uint8_t length = 128;
        char line[length] = {'\0'};
        file.readBytesUntil('\r', line, length);
        file.close();

        CSV csv;
        ota_chunk_info_t info;
        ret = csv.Decode(line, &info);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ INDEX FROM THE FIRST LINE: %s", line);
        }
        else
        {
            *index = info.Index;
        }
        return ret;
    }

    Status FindChunkInfoService(const ota::mcu_e mcuType, const uint8_t idx, ota_chunk_info_t* info)
    {
        ASSERT((info != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        CSV csv;
        Status ret(Status::Code::BAD_NOT_FOUND);
        File file = mcuType == ota::mcu_e::MCU1 ? esp32FS.Open(OTA_CHUNK_PATH_ESP32, "r", false)
                                                : esp32FS.Open(OTA_CHUNK_PATH_MEGA,  "r", false);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN OTA CHUNK INFO FILE");
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }

        int left = 0;
        int right = file.size() - 1;
        int middle = (left + right) / 2;

        bool isPositionZero = false;
        bool hasFoundIndex = false;

        while (left <= right)
        {
            if (isPositionZero == true)
            {
                LOG_ERROR(logger, "FAILED TO FIND GIVEN INDEX: %u", idx);
                goto ON_EXIT;
            }
            
            middle = (left + right) / 2;
            file.seek(middle);

            while ((file.position() > 0) && (file.peek() != '\n'))
            {
                file.seek(file.position() - 1);
            }
            
            if (file.position() == 0)
            {
                isPositionZero = true;
            }
            else
            {
                file.read();
            }

            const uint8_t length = 128;
            char line[length] = {'\0'};
            file.readBytesUntil('\n', line, length);
            
            char* carriageReturn = strrchr(line, '\r');
            if (carriageReturn != NULL)
            {
                *carriageReturn = '\0';
            }
            
            ret = csv.Decode(line, info);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DECODE LINE: %s", line);
                ret = Status::Code::BAD_END_OF_STREAM;
                goto ON_EXIT;
            }
            
            if (info->Index == idx)
            {
                hasFoundIndex = true;
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
        if (hasFoundIndex == false)
        {
            ret = Status::Code::BAD_NOT_FOUND;
        }
        
        return ret;
    }
}