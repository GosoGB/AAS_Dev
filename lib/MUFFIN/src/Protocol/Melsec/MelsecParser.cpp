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
        size_t calculateLength = length - HEADER_SIZE - ENDCODE_SIZE;          // 전체 길이 - 헤더 길이 - ENDCODE
        if (receiveLength == 0 || (calculateLength != receiveLength)) 
        {
            LOG_ERROR(logger,"RECEIVE LENGTH ERROR, receiveLength : %u, calculateLength :%u ",receiveLength, calculateLength)   
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }


        const uint8_t* dataStart = frame + HEADER_SIZE + ENDCODE_SIZE;
        size_t dataSize = receiveLength - ENDCODE_SIZE;
        if (isBit) 
        {
            // Bit 모드: 1문자 = 1비트 ('0' or '1')
            for (size_t i = 0; i < receiveLength; ++i) 
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
            if (receiveLength % 4 != 0) 
            {
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            }
    
            for (size_t i = 0; i < receiveLength; i += 4) 
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
        uint16_t dataLength = static_cast<uint16_t>(frame[7]) | (static_cast<uint16_t>(frame[8]) << 8);

        size_t expectedTotal = HEADER_SIZE + dataLength;

        if (length < expectedTotal) 
        {
            LOG_ERROR(logger, "RECEIVE LENGTH ERROR, dataLength : %u, total length: %zu", dataLength, length);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        // 3. 데이터 파싱
        const uint8_t* dataStart = frame + HEADER_SIZE + ENDCODE_SIZE;
        const size_t dataSize = dataLength - ENDCODE_SIZE;

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

} 