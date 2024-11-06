/**
 * @file HexParser.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Intel Hex 형식의 데이터를 파싱 하는 클래스를 정의합니다.
 * 
 * @date 2024-11-15
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin { namespace ota {

    class HexParser
    {
    public:
        HexParser() {}
        ~HexParser() {}

    private:
        /**
         * @brief Extract the data from a hex Record
         * @example Hex Record: ":100000000C9434000C9446000C9446000C9446006A"
         *      : => Record start
         *      10 => Data length (16 bytes for most records)
         *      0000 => Start Address
         *      00 => Record Type (00 = Data Record)
         *      0C 94 34 00 0C 94 46 00 0C 94 46 00 0C 94 46 00 => Data to be extracted
         *      6A => Checksum
         *
         * @param buffer Raw hex Record
         * @param data To store the parsed result
         * @param start Starting index of Data in Record
         * @param end Ending index of Data in Record  
         */
        void extractData(char buffer[], char data[], int start, int end)
        {
            int idx = 0;
            int size = (end - start) + 1;

            while (idx < size)
            {
                data[idx] = buffer[start + idx];
                idx++;
            }
            data[idx] = '\0';
        }

    public:
        /**
         * @brief Parse an entire .hex file for data
         * @details It gives out a 'page' of data, containing 
         *          all the Data parsed from each of the records
         *          from the .hex file.
         * 
         * @param filepath the .hex file to be parsed
         * @param page To store the parsed result
         * @param blockCount Total no. of blocks (128 bytes each)
         * @return Status 
         */
        Status Parse(const char* filepath, uint8_t page[], int* blockCount, size_t* byteCount)
        {
            ESP32FS* esp32fs = ESP32FS::CreateInstanceOrNULL();
            esp32fs->Begin(true);
            
            File file = esp32fs->Open(filepath, "r");
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN HEX FILE: %s", filepath);
                return Status(Status::Code::BAD_NOT_FOUND);
            }

            int idx = 0;
            while (true)
            {
                char currentLine[64];
                char previousLine[64];
                
                size_t len = file.readBytesUntil('\n', currentLine, sizeof(currentLine));
                if (strcmp(previousLine, currentLine) == 0 || len == 0)
                {
                    break;
                }

                currentLine[len++] = '\n';
                currentLine[len++] = '\0';

                char buffer[strlen(currentLine) - 1];
                strcpy(buffer, currentLine);

                if (strlen(buffer) > 12)
                {
                    int start = 9;
                    int end = strlen(buffer) - 4;
                    int size = (end - start) + 1;

                    char raw_data[size];
                    extractData(buffer, raw_data, start, end);

                    char byteBuffer[3];
                    byteBuffer[2] = '\0';
                    for (int i = 0; i < strlen(raw_data) - 1; i += 2)
                    {
                        byteBuffer[0] = raw_data[i];
                        byteBuffer[1] = raw_data[i + 1];
                        page[idx] = strtol(byteBuffer, 0, 16);
                        idx++;
                    }
                }

                strcpy(previousLine, currentLine);
            }
        
            while (idx % 257 != 0)
            {
                page[idx] = 0xFF;
                idx++;
            }

            *blockCount = (idx - 1) / 256;
            *byteCount = (idx - 1);
            LOG_DEBUG(logger, "Block count: %d", *blockCount);
            LOG_DEBUG(logger, "Byte count: %d", *byteCount);
            return Status(Status::Code::GOOD);
        }
    };
}}