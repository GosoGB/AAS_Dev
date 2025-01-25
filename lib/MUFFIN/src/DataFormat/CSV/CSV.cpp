/**
 * @file CSV.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief CSV 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 정의합니다.
 * 
 * @date 2025-01-25
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Logger/Logger.h"
#include "CSV.h"




namespace muffin {

    Status CSV::Decode(const char* input, init_cfg_t* output)
    {
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        const uint8_t COLUMN_COUNT = 3;
        const uint8_t COLUMN_WIDTH = 2;
        char buffer[COLUMN_COUNT][COLUMN_WIDTH];
        
        Status ret = parse(input, COLUMN_COUNT, COLUMN_WIDTH, reinterpret_cast<char**>(buffer));
        if (ret == Status::Code::BAD_NO_DATA)
        {
            LOG_ERROR(logger, ret.c_str());
            return ret;
        }
        else if (ret == Status::Code::UNCERTAIN_DATA_SUBNORMAL)
        {
            LOG_WARNING(logger, ret.c_str());
        }
        
        output->PanicResetCount   = Convert.ToInt8(buffer[0]);
        output->HasPendingJARVIS  = Convert.ToInt8(buffer[1]);
        output->HasPendingUpdate  = Convert.ToInt8(buffer[2]);

        if ((-1 < output->PanicResetCount   && output->PanicResetCount  < 6) ||
            (-1 < output->HasPendingJARVIS  && output->HasPendingJARVIS < 2) ||
            (-1 < output->HasPendingUpdate  && output->HasPendingUpdate < 2))
        {
            ret = Status::Code::BAD_DATA_LOST;
            LOG_ERROR(logger, ret.c_str());
            return ret;
        }
        
        return ret;
    }

    Status CSV::Encode(const init_cfg_t input, const size_t length, char output[])
    {
        ASSERT((length > 6), "BUFFER LENGTH TOO SMALL");
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        memset(output, '\0', length);
        snprintf(output, length, "%u,%u,%u",
            input.PanicResetCount,
            input.HasPendingJARVIS,
            input.HasPendingUpdate);

        init_cfg_t readback;
        Status ret = Decode(output, &readback);
        if ((input.PanicResetCount   != readback.PanicResetCount)  ||
            (input.HasPendingJARVIS  != readback.HasPendingJARVIS) ||
            (input.HasPendingUpdate  != readback.HasPendingUpdate) ||
            (ret != Status::Code::GOOD))
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        
        return Status(Status::Code::GOOD);
    }

    Status CSV::parse(const char* input, const uint8_t columnCount, const uint8_t columnWidth, char** output)
    {
        Status ret(Status::Code::GOOD);

        if ((input != nullptr) && (strlen(input) == 0))
        {
            ret = Status::Code::BAD_NO_DATA;
            return ret;
        }

        char (*buffer)[columnWidth] = reinterpret_cast<char (*)[columnWidth]>(output);
        for (uint8_t column = 0; column < columnCount; ++column)
        {
            memset(buffer[column], '\0', columnWidth);
        }

        uint8_t idx = 0;
        const char* start = input;
        const char* end   = input;

        while (*end != '\0' && idx < columnCount)
        {
            if (*end == ',')
            {
                uint8_t length = end - start;
                if (length >= columnWidth)
                {
                    ret = Status::Code::UNCERTAIN_DATA_SUBNORMAL;
                    length = columnWidth - 1;
                }

                strncpy(buffer[idx], start, length);
                buffer[idx][length] = '\0';
                start = end + 1;
                idx++;
            }
            end++;
        }

        if (idx < columnCount)
        {
            uint8_t length = end - start;
            if (length >= columnWidth)
            {
                ret = Status::Code::UNCERTAIN_DATA_SUBNORMAL;
                length = columnWidth - 1;
            }

            strncpy(buffer[idx], start, length);
            buffer[idx][length] = '\0';
            idx++;
        }

        return ret;
    }
}