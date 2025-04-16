/**
 * @file MelsecCommonHeader.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#pragma once

#include <string>



namespace muffin {

    typedef enum class MelsecCommand
        : uint8_t
    {
        NOP                 = 0,
        BATCH_READ          = 1,
        BATCH_WRITE         = 2
    } melsec_command_e;

    struct MelsecCommonHeader 
    {
        uint16_t SubHeader = 0x5000;
        uint8_t NetworkNumber = 0x00;
        uint8_t PcNumber = 0xFF;
        uint16_t IoNumber = 0x03FF;
        uint8_t StationNumber = 0x00;
        uint8_t MonitoringTimer = 0x08;
    };

}