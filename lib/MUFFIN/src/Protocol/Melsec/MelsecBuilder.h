/**
 * @file MelsecBuilder.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */




#pragma once

#include <string>
#include "include/MelsecCommonHeader.h"
#include "IM/Node/Include/TypeDefinitions.h"


namespace muffin{

    class MelsecBuilder
    {
    private:
        /* data */
    public:
        MelsecBuilder();
        ~MelsecBuilder();

    public:
        size_t BuildNopCommand(jvs::df_e dataFormat, MelsecCommonHeader commonHeader, uint8_t* frame);
        size_t BuildCommonHeader(jvs::df_e dataFormat, MelsecCommonHeader commonHeader, uint8_t* frame);
        size_t BuildRequestData(jvs::df_e dataFormat, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame);

    private:
        size_t buildAsciiCommonHeader(MelsecCommonHeader commonHeader, uint8_t* frame);
        size_t buildBinaryCommonHeader(MelsecCommonHeader commonHeader, uint8_t* frame);

        size_t buildAsciiRequestData(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame);
        size_t buildBinaryRequestData(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame);
        
    private:
        size_t buildAsciiRequestCommand(melsec_command_e command, uint8_t* frame);
        // size_t buildBinaryRequestCommand(melsec_command_e command, uint8_t* frame);

        size_t buildAsciiRequestSubCommand(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame);
        // size_t buildBinaryRequestSubCommand(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame);

    private:
        std::string getDeviceCodeASCII(jvs::node_area_e area);
        uint8_t getDeviceCodeBinary(jvs::node_area_e area);
        bool isHexMemory(const jvs::node_area_e area);
    };
    
    
    
}