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
        BATCH_READ          = 1, // @lsj batch read 값이 프로토콜 내부에 활용될 수 있는 값이라면 그 값을 할당시켜줘도 좋습니다.
        BATCH_WRITE         = 2
    } melsec_command_e;

    struct MelsecCommonHeader 
    {// @lsj 변하지 않는 값이라면 static const를 붙여주는 게 어떨까요?
        static const uint16_t SubHeader = 0x5000;
        static const uint8_t NetworkNumber = 0x00;
        static const uint8_t PcNumber = 0xFF;
        static const uint16_t IoNumber = 0x03FF;
        static const uint8_t StationNumber = 0x00;
        static const uint8_t MonitoringTimer = 0x08;
    };

}