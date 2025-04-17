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
        size_t BuildReadRequestDataASCII(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, uint8_t* frame);
        size_t BuildWriteRequestDataASCII(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const uint16_t data[], uint8_t* frame);
    public:   
        size_t BuildReadRequestDataBinary(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, uint8_t* frame);
        size_t BuildWriteRequestDataBinary(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const uint16_t data[], uint8_t* frame);
        
    private:
        size_t buildCommonHeaderASCII(MelsecCommonHeader commonHeader, uint8_t* frame);
        size_t buildRequestDataASCII(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame);
        
        size_t buildCommonHeaderBinary(MelsecCommonHeader commonHeader, uint8_t* frame);
        size_t buildRequestDataBinary(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame);
        
    private:
        size_t buildRequestCommandASCII(melsec_command_e command, uint8_t* frame);
        size_t buildRequestSubCommandASCII(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame);
        
        size_t buildRequestCommandBinary(melsec_command_e command, uint8_t* frame);
        size_t buildRequestSubCommandBinary(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame);

    private:
        std::string getDeviceCodeASCII(jvs::node_area_e area);
        uint8_t getDeviceCodeBinary(jvs::node_area_e area);
        bool isHexMemory(const jvs::node_area_e area);
    };
    
    
    
}