/**
 * @file MelsecClient.h
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec Client 클래스를 정의합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#pragma once


#include <vector>

#include <Arduino.h>
#include <WiFi.h>
#include "JARVIS/Include/TypeDefinitions.h"
#include "include/MelsecCommonHeader.h"
#include "MelsecBuilder.h"
#include "TCPTransport.h"



namespace muffin
{
    class MelsecClient
    {
    public:
        MelsecClient();
        virtual ~MelsecClient();
    
    public:
        // Initialize communication with the PLC
        bool Begin(const char *ip, uint16_t port, jvs::ps_e series = jvs::ps_e::QL_SERIES);
        bool Connected();

        // Set communication mode (ASCII or BINARY)
        void SetHeader(uint8_t networkNo, uint8_t pcNo, uint16_t ioNo, uint8_t stationNo);
        void SetDataFormat(jvs::df_e format) { mDataFormat = format; }
        
        // Write operations
        bool WriteWords(jvs::node_area_e area, uint32_t address, int wordCount, const uint16_t data[]);
        bool WriteWord(jvs::node_area_e area, uint32_t address, uint16_t word);
        bool WriteBit(jvs::node_area_e area, uint32_t address, uint8_t value);
        bool WriteBits(jvs::node_area_e area, uint32_t address, int count, const bool *values);

        // Read operations
        int ReadWords(jvs::node_area_e area, uint32_t address, int wordCount, uint16_t buffer[]);
        int ReadBits(jvs::node_area_e area, uint32_t address, int count, bool *buffer);

    private:
        std::string batchReadWrite(jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit = false, const std::string &dataToWrite = "");


        std::string buildAsciiCommonHeader(); // 내부 ASCII 헤더 생성 함수
        int buildBinaryCommonHeader(uint8_t* frame);

        std::string buildAsciiRequestData(jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit);
        int buildBinaryRequestData(uint8_t* frame, size_t index, jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit);

        // Low-level communication
        std::string sendAndReceive(const std::string &command);
        int sendAndReceive(const std::string &command, uint16_t buffer[]);

        // Data conversion helpers
        int hexStringToWords(const std::string &hexStr, uint16_t buffer[]);
        int wordsToHexString(const uint16_t data[], int wordCount, std::string &hexStr);
        int wordArrayToByteArray(const uint16_t words[], uint8_t bytes[], int wordCount);

        std::string fitStringToWords(const std::string &input, int wordCount);
        std::string stringToHexASCII(const std::string &input);
        std::string extractAsciiData(const std::string &response);
        std::string extractBinaryData(const std::string &response);

        uint8_t getDeviceCodeBinary(jvs::node_area_e area);
        std::string getDeviceCodeASCII(jvs::node_area_e area);

        bool isHexMemory(const jvs::node_area_e type);
       
    private:
        // 기존 변수 외에 ASCII 헤더 구성용 변수 추가
        TCPTransport mMelsecTCP;
        MelsecBuilder mMelsecBuilder;
    private:
        MelsecCommonHeader mCommonHeader;
        uint16_t mPort;
        const char *mIP;
        jvs::ps_e mPlcSeries;
        jvs::df_e mDataFormat;
    };
} 
