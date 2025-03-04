/**
 * @file EspOTA.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief ESP32 전용 펌웨어 업데이트 클래스를 정의합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <Update.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/TypeDefinitions.h"
#include "StrategyESP32.h"



namespace muffin { namespace ota {

    Status StrategyESP32::Init(const fw_info_t& info)
    {
        ASSERT((info.Chunk.Count != 0), "CHUNK COUNT MUST BE GREATER THAN 0");
        
        if (Update.begin(info.TotalSize) == false)
        {
            LOG_ERROR(logger, "OUT OF MEMORY: TOO LARGE FOR THE APP PARTITION");
            return Status(Status::Code::BAD_INITIAL_VALUE_OUT_OF_RANGE);
        }

        return Status(Status::Code::GOOD);
    }

    Status StrategyESP32::Write(const size_t size, uint8_t byteArray[])
    {
        ASSERT((Update.remaining() != 0), "NO REMAINING BYTES TO WRITE");
        ASSERT((Update.isFinished() != true), "UPDATE HAS BEEN FINISHED");

        if (Update.write(byteArray, size) != size)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        const float progress = static_cast<float>(Update.progress()) / static_cast<float>(Update.size());
        LOG_INFO(logger, "Progress: %.2f%%", 100.0f * progress);

        const uint8_t errorCode = Update.getError();
        return processErrorCode(errorCode);
    }

    Status StrategyESP32::TearDown()
    {
        if (Update.end() == false)
        {
            const uint8_t errorCode = Update.getError();
            const Status ret = processErrorCode(errorCode);
            if (ret.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE: %s", ret.c_str());
                return ret;
            }
        }

        LOG_INFO(logger, "Updated successfully");
        return Status(Status::Code::GOOD);
    }

    Status StrategyESP32::processErrorCode(const uint8_t errorCode) const
    {
        switch (errorCode)
        {
        case 0:
            return Status(Status::Code::GOOD);
        case 1:
            LOG_ERROR(logger, "FAILED TO WRITE INTO APP PARTITION");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 2:
            LOG_ERROR(logger, "FAILED TO ERASE FROM APP PARTITION");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 3:
            LOG_ERROR(logger, "FAILED TO READ FROM APP PARTITION");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 4:
            LOG_ERROR(logger, "APP PARTITION IS OUT OF MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 5:
            LOG_ERROR(logger, "INVALID SIZE WAS GIVEN");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        case 6:
            LOG_ERROR(logger, "STREAM READ TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 7:
            LOG_ERROR(logger, "INVALID MD5 CHECK VALUE");
            return Status(Status::Code::BAD_DATA_LOST);
        case 8:
            LOG_ERROR(logger, "INVALID MAGIC BYTE");
            return Status(Status::Code::BAD_DATA_LOST);
        case 9:
            LOG_ERROR(logger, "FAILED TO ACTIVATE THE FIRMWARE");
            return Status(Status::Code::BAD_STATE_NOT_ACTIVE);
        case 10:
            LOG_ERROR(logger, "NO APP PARTITION AVAILABLE");
            return Status(Status::Code::BAD_INVALID_STATE);
        case 11:
            LOG_ERROR(logger, "INVALID ARGUMENT WAS GIVEN");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        case 12:
            LOG_ERROR(logger, "UPDATE HAS BEEN ABORTED");
            return Status(Status::Code::BAD_OPERATION_ABANDONED);
        default:
            LOG_ERROR(logger, "UNDEFINED ERROR CODE: %u", errorCode);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }
}}