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
#include "MelsecParser.h"
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
        bool WriteWords(jvs::node_area_e area, uint32_t address, size_t count, const uint16_t value[]);
        bool WriteWord(jvs::node_area_e area, uint32_t address, uint16_t value);
        bool WriteBit(jvs::node_area_e area, uint32_t address, uint16_t value);
        bool WriteBits(jvs::node_area_e area, uint32_t address, size_t count, const uint16_t value[]);

        // Read operations
        int ReadWords(jvs::node_area_e area, uint32_t address, size_t count, uint16_t buffer[]);
        int ReadBits(jvs::node_area_e area, uint32_t address, size_t count, uint16_t buffer[]);

    private:
        // 기존 변수 외에 ASCII 헤더 구성용 변수 추가
        TCPTransport mMelsecTCP;
        MelsecBuilder mMelsecBuilder;
        MelsecParser mMelsecParser;

    private:
        MelsecCommonHeader mCommonHeader;
        uint16_t mPort;
        const char *mIP;
        jvs::ps_e mPlcSeries;
        jvs::df_e mDataFormat;
    };
} 
