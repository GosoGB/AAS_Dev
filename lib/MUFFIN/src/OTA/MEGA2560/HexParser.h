/**
 * @file HexParser.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Intel Hex 형식의 데이터를 파싱 하는 클래스를 선언합니다.
 * 
 * @date 2025-02-06
 * @version 1.3.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024-2025
 */




#if defined(MODLINK_T2)



#pragma once

#include <string>
#include <vector>

#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Include/TypeDefinitions.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin { namespace ota {

    class HexParser
    {
    public:
        HexParser() {}
        virtual ~HexParser() {}
    public:
        /**
         * @brief ATmega2560 펌웨어 파일 chunk를 페이지 단위로 파싱하여 출력합니다.
         * @example Hex 레코드: ":100000000C9434000C9446000C9446000C9446006A"
         *      : => Record start
         *      10 => Data length (16 bytes for most records)
         *      0000 => Start Address
         *      00 => Record Type (00 = Data Record)
         *      0C 94 34 00 0C 94 46 00 0C 94 46 00 0C 94 46 00 => Data to be extracted
         *      6A => Checksum
         *
         * @param chunk ATmega2560 펌웨어 파일을 잘라 만든 Intel Hex 레코드 덩어리
         * @param pages 256 bytes 크기로 파싱된 Intel Hex 파일
         * @return Status 
         */
        Status Parse(std::string& chunk);
        size_t GetPageCount() const;
        page_t GetPage();
        void RemovePage();
    private:
        static constexpr uint8_t START_INDEX = 9;
        static constexpr uint8_t CR_LF_LENGTH = 2;
        static constexpr uint8_t CHECKSUM_CHAR_LENGTH = 2;
    private:
        std::string mReceivedData;
        std::vector<std::string> mHexRecords;
        std::vector<page_t> mPages;
    };
}}



#endif