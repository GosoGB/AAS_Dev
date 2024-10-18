/**
 * @file Convert.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다양한 데이터 타입 간의 변환을 지원하는 유틸리티 클래스를 정의합니다.
 * 
 * @date 2024-10-17
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <cstdlib>
#include <cerrno>
#include <cfloat>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ConvertClass.h"



namespace muffin {

    ConvertClass Convert;
    
    int8_t ConvertClass::ToInt8(const std::string& input)
    {
        return static_cast<int8_t>(ToInt64(input));
    }

    int16_t ConvertClass::ToInt16(const std::string& input)
    {
        return static_cast<int16_t>(ToInt64(input));
    }

    int32_t ConvertClass::ToInt32(const std::string& input)
    {
        return static_cast<int32_t>(ToInt64(input));
    }

    int64_t ConvertClass::ToInt64(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const int64_t output = std::strtoll(input.c_str(), &endPointer, 10);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return INT64_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return INT64_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return INT64_MAX;
        }
        else
        {
            return output;
        }
    }

    uint8_t ConvertClass::ToUInt8(const std::string& input)
    {
        return static_cast<uint8_t>(ToUInt64(input));
    }

    uint16_t ConvertClass::ToUInt16(const std::string& input)
    {
        return static_cast<uint16_t>(ToUInt64(input));
    }

    uint32_t ConvertClass::ToUInt32(const std::string& input)
    {
        return static_cast<uint32_t>(ToUInt64(input));
    }

    uint32_t ConvertClass::ToUInt32(const time_t input)
    {
        return static_cast<uint32_t>(input);
    }

    uint64_t ConvertClass::ToUInt64(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const uint64_t output = std::strtoull(input.c_str(), &endPointer, 10);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return UINT64_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return UINT64_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return UINT64_MAX;
        }
        else
        {
            return output;
        }
    }

    float ConvertClass::ToFloat(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const float output = std::strtof(input.c_str(), &endPointer);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return FLT_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return FLT_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return FLT_MAX;
        }
        else
        {
            return output;
        }
    }

    double ConvertClass::ToDouble(const std::string& input)
    {
        char* endPointer;
        errno = 0;

        const double output = std::strtod(input.c_str(), &endPointer);
        
        if (endPointer == input.c_str())
        {
            LOG_ERROR(logger, "NONCONVERTIBLE STRING: %s", input.c_str());
            return DBL_MAX;
        }
        else if (*endPointer != '\0')
        {
            LOG_ERROR(logger, "PARTIALLY CONVERTED: %s", input.c_str());
            return DBL_MAX;
        }
        else if (errno == ERANGE)
        {
            LOG_ERROR(logger, "OUT OF RANGE: %s", input.c_str());
            return DBL_MAX;
        }
        else
        {
            return output;
        }
    }
}