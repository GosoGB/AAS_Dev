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
        NOP                 = 0, // @lsj NOP가 뭐의 약자인지...? 왠만하면 이넘이나 클래스 등의 이름은 풀 네임을 써주는 게 좋아요 인스턴스 레벨에서는 약자로 써도 좋은데
        BATCH_READ          = 1, // @lsj batch read 값이 프로토콜 내부에 활용될 수 있는 값이라면 그 값을 할당시켜줘도 좋습니다.
        BATCH_WRITE         = 2
    } melsec_command_e;

    struct MelsecCommonHeader 
    {// @lsj 변하지 않는 값이라면 static const를 붙여주는 게 어떨까요?
        uint16_t SubHeader = 0x5000;
        uint8_t NetworkNumber = 0x00;
        uint8_t PcNumber = 0xFF;
        uint16_t IoNumber = 0x03FF;
        uint8_t StationNumber = 0x00;
        uint8_t MonitoringTimer = 0x08;
    };

}