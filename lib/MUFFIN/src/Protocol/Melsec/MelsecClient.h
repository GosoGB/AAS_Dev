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
#include "JARVIS/Include/TypeDefinitions.h"
#include "Include/MelsecCommonHeader.h"
#include "MelsecBuilder.h"
#include "MelsecParser.h"
#if defined(MT11)
    #include "Network/Ethernet/W5500/EthernetClient.h"
#else
    #include "WiFi.h"
#endif


namespace muffin
{

    class MelsecClient
    {
    public:
#if defined(MT11)
        MelsecClient(W5500& interface, const w5500::sock_id_e sock_id);
#else
        MelsecClient();
#endif
        virtual ~MelsecClient();
    
    public:
        // Initialize communication with the PLC
        bool Begin(IPAddress ip, uint16_t port, jvs::ps_e series = jvs::ps_e::QL_SERIES);
        bool Connected(); // @lsj IsConnected 같은 식으로 이름을 짓는 게 어떨까요?
        void Close();

        // Set communication mode (ASCII or BINARY)
        // void SetHeader(uint8_t networkNo, uint8_t pcNo, uint16_t ioNo, uint8_t stationNo);
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
        int sendAndReceive(const uint8_t *cmd, int length, uint8_t *responseBuf);


    private:
        // 기존 변수 외에 ASCII 헤더 구성용 변수 추가
        MelsecBuilder mMelsecBuilder;
        MelsecParser mMelsecParser;
    public:
    #if defined(MT11)
        w5500::EthernetClient* mClient = nullptr;
        uint8_t mReqFrame[512];
        uint8_t mRespFrame[1024];
    #else
        WiFiClient* mClient = nullptr;
        uint8_t mReqFrame[256];
        uint8_t mRespFrame[512];
    #endif

        
    private:
        bool mIsConnected = false;
        MelsecCommonHeader mCommonHeader;
        uint16_t mPort;
        IPAddress mIP;
        jvs::ps_e mPlcSeries;
        jvs::df_e mDataFormat;

    };

    extern MelsecClient* embededMelsecClient;
    extern MelsecClient* link1MelsecClient;
} 
