/**
 * @file MelsecParser.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */




#include <string.h> 
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"

#include "MelsecParser.h"



namespace muffin
{
    MelsecParser::MelsecParser()
    {

    }

    MelsecParser::~MelsecParser()
    {

    }

    Status MelsecParser::ParseReadResponseASCII(const uint8_t* frame, size_t length, bool isBit, uint16_t* outBuffer)
    {
        constexpr size_t HEADER_SIZE = 18;
        constexpr size_t ENDCODE_SIZE = 4;

        if (length < HEADER_SIZE) 
        {
            LOG_ERROR(logger,"HEADER SIZE ERROR"); 
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        size_t outCount = 0;
        char lenBuf[5] = {0};
        memcpy(lenBuf, &frame[14], 4);
        size_t receiveLength = strtoul(lenBuf, nullptr, 16);
        size_t calculateLength = length - HEADER_SIZE ;          // 전체 길이 - 헤더 길이
        if (receiveLength == 0 || (calculateLength != receiveLength)) 
        {
            LOG_ERROR(logger,"RECEIVE LENGTH ERROR, length : %u, receiveLength : %u, calculateLength :%u ",length, receiveLength, calculateLength);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        char endCodeStr[5] = {0};
        memcpy(endCodeStr, &frame[HEADER_SIZE], 4);
        uint16_t endCode = static_cast<uint16_t>(strtoul(endCodeStr, nullptr, 16));

        if (endCode != 0x0000) 
        {
            LOG_ERROR(logger, "MELSEC END CODE ERROR: %04X", endCode);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);  // 또는 적절한 오류 코드로 변경
        }

        const uint8_t* dataStart = frame + HEADER_SIZE + ENDCODE_SIZE;
        const size_t dataSize = receiveLength - ENDCODE_SIZE;
        if (isBit) 
        {
            // Bit 모드: 1문자 = 1비트 ('0' or '1')
            for (size_t i = 0; i < dataSize; ++i) 
            {
                uint8_t ch = dataStart[i];
                if (ch == '0') 
                {
                    outBuffer[outCount++] = 0;
                } 
                else if (ch == '1') 
                {
                    outBuffer[outCount++] = 1;
                } 
                else 
                {
                    return Status(Status::Code::BAD_INVALID_ARGUMENT);
                }
            }
        } 
        else 
        {
            // Word 모드: 4문자 = 1 WORD
            if (dataSize % 4 != 0) 
            {
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            }
    
            for (size_t i = 0; i < dataSize; i += 4) 
            {
                char wordStr[5] = {0};
                memcpy(wordStr, &dataStart[i], 4);
    
                uint16_t word = static_cast<uint16_t>(strtoul(wordStr, nullptr, 16));
                outBuffer[outCount++] = word;
            }
        }
    
        return Status(Status::Code::GOOD);
    }

    Status MelsecParser::ParseReadResponseBinary(const uint8_t* frame, size_t length, bool isBit, uint16_t* outBuffer)
    {
        constexpr size_t HEADER_SIZE = 9;
        constexpr size_t ENDCODE_SIZE = 2;

        size_t outCount = 0;

        if (length < HEADER_SIZE + ENDCODE_SIZE) 
        {
            LOG_ERROR(logger, "FRAME TOO SHORT");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        // 1. Length 필드 (frame[7], frame[8]) - Little Endian
        size_t receiveLength = static_cast<uint16_t>(frame[7]) | (static_cast<uint16_t>(frame[8]) << 8);
        size_t calculateLength = length - HEADER_SIZE ;          // 전체 길이 - 헤더 길이
        if (receiveLength == 0 || (calculateLength != receiveLength)) 
        {
            LOG_ERROR(logger,"RECEIVE LENGTH ERROR, length : %u, receiveLength : %u, calculateLength :%u ",length, receiveLength, calculateLength); 
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        // 2. End Code 확인
        uint16_t endCode = static_cast<uint16_t>(frame[HEADER_SIZE]) | (static_cast<uint16_t>(frame[HEADER_SIZE + 1]) << 8);

        if (endCode != 0x0000) 
        {
            LOG_ERROR(logger, "MELSEC END CODE ERROR: 0x%04X", endCode);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);  // 또는 적절한 오류 코드
        }

        // 3. 데이터 파싱
        const uint8_t* dataStart = frame + HEADER_SIZE + ENDCODE_SIZE;
        const size_t dataSize = receiveLength - ENDCODE_SIZE;

        if (isBit) 
        {
            // 1 바이트 = 1 비트
            for (size_t i = 0; i < dataSize; ++i) 
            {
                if (dataStart[i] == 0x00 || dataStart[i] == 0x01) 
                {
                    outBuffer[outCount++] = dataStart[i];
                } 
                else 
                {
                    LOG_ERROR(logger, "INVALID BIT VALUE: 0x%02X", dataStart[i]);
                    return Status(Status::Code::BAD_INVALID_ARGUMENT);
                }
            }
        } 
        else 
        {
            // 2 바이트 = 1 WORD
            if (dataSize % 2 != 0) 
            {
                LOG_ERROR(logger, "WORD DATA LENGTH NOT ALIGNED: %zu", dataSize);
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            }

            for (size_t i = 0; i < dataSize; i += 2) 
            {
                uint16_t word = static_cast<uint16_t>(dataStart[i]) |
                                (static_cast<uint16_t>(dataStart[i + 1]) << 8);
                outBuffer[outCount++] = word;
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status MelsecParser::ParseWriteResponseASCII(const uint8_t* frame, size_t length, bool isBit)
    {
        constexpr size_t HEADER_SIZE = 18;
        constexpr size_t ENDCODE_SIZE = 4;

        if (length < HEADER_SIZE) 
        {
            LOG_ERROR(logger,"HEADER SIZE ERROR"); 
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        size_t outCount = 0;
        char lenBuf[5] = {0};
        memcpy(lenBuf, &frame[14], 4);
        size_t receiveLength = strtoul(lenBuf, nullptr, 16);
        size_t calculateLength = length - HEADER_SIZE ;          // 전체 길이 - 헤더 길이
        if (receiveLength == 0 || (calculateLength != receiveLength)) 
        {
            LOG_ERROR(logger,"RECEIVE LENGTH ERROR, length : %u, receiveLength : %u, calculateLength :%u ",length, receiveLength, calculateLength);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        char endCodeStr[5] = {0};
        memcpy(endCodeStr, &frame[HEADER_SIZE], 4);
        uint16_t endCode = static_cast<uint16_t>(strtoul(endCodeStr, nullptr, 16));

        if (endCode != 0x0000) 
        {
            LOG_ERROR(logger, "MELSEC END CODE ERROR: %04X", endCode);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);  // 또는 적절한 오류 코드로 변경
        }
    
        return Status(Status::Code::GOOD);
    }

    Status MelsecParser::ParseWriteResponseBinary(const uint8_t* frame, size_t length, bool isBit)
    {
        constexpr size_t HEADER_SIZE = 9;
        constexpr size_t ENDCODE_SIZE = 2;

        size_t outCount = 0;

        if (length < HEADER_SIZE + ENDCODE_SIZE) 
        {
            LOG_ERROR(logger, "FRAME TOO SHORT");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        // 1. Length 필드 (frame[7], frame[8]) - Little Endian
        size_t receiveLength = static_cast<uint16_t>(frame[7]) | (static_cast<uint16_t>(frame[8]) << 8);
        size_t calculateLength = length - HEADER_SIZE ;          // 전체 길이 - 헤더 길이
        if (receiveLength == 0 || (calculateLength != receiveLength)) 
        {
            LOG_ERROR(logger,"RECEIVE LENGTH ERROR, length : %u, receiveLength : %u, calculateLength :%u ",length, receiveLength, calculateLength); 
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        // 2. End Code 확인
        uint16_t endCode = static_cast<uint16_t>(frame[HEADER_SIZE]) | (static_cast<uint16_t>(frame[HEADER_SIZE + 1]) << 8);

        if (endCode != 0x0000) 
        {
            LOG_ERROR(logger, "MELSEC END CODE ERROR: 0x%04X", endCode);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);  // 또는 적절한 오류 코드
        }

        return Status(Status::Code::GOOD);
    }
} 