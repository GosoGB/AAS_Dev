/**
 * @file Command.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief AVR068: STK500 Communication Protocol 문서에 정의된 명령 및 응답 코드를 정의합니다.
 * @note STK500 프로토콜 버전 2에 대응되는 명령 및 응답 코드입니다. 버전 1과 호환되지 않습니다.
 * @ref https://github.com/arduino/Arduino-stk500v2-bootloader
 * 
 * @date 2024-11-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin { namespace ota {

    /**
     * @brief STK500 메시지 형식에 공통적으로 사용되는 상수
     */
    constexpr uint8_t MESSAGE_START               = 0x1B;
    constexpr uint8_t TOKEN                       = 0x0E;

    /**
     * @brief STK500 프로토콜 일반에 사용되는 명령 코드
     */
    constexpr uint8_t CMD_SIGN_ON                 =  0x01;
    constexpr uint8_t CMD_SET_PARAMETER           =  0x02;
    constexpr uint8_t CMD_GET_PARAMETER           =  0x03;
    constexpr uint8_t CMD_SET_DEVICE_PARAMETERS   =  0x04;
    constexpr uint8_t CMD_OSCCAL                  =  0x05;
    constexpr uint8_t CMD_LOAD_ADDRESS            =  0x06;
    constexpr uint8_t CMD_FIRMWARE_UPGRADE        =  0x07;

    /**
     * @brief STK500 응답에서 사용되는 상태 코드
     */
    constexpr uint8_t STATUS_CMD_OK               = 0x00;    // GOOD
    constexpr uint8_t STATUS_CMD_TOUT             = 0x80;    // WARNING
    constexpr uint8_t STATUS_RDY_BSY_TOUT         = 0x81;    // WARNING
    constexpr uint8_t STATUS_SET_PARAM_MISSING    = 0x82;    // WARNING
    constexpr uint8_t STATUS_CMD_FAILED           = 0xC0;    // BAD
    constexpr uint8_t STATUS_CKSUM_ERROR          = 0xC1;    // BAD
    constexpr uint8_t STATUS_CMD_UNKNOWN          = 0xC9;    // BAD

    /**
     * @brief STK500 프로토콜 설정 코드
     */
    constexpr uint8_t PARAM_BUILD_NUMBER_LOW      = 0x80;
    constexpr uint8_t PARAM_BUILD_NUMBER_HIGH     = 0x81;
    constexpr uint8_t PARAM_HW_VER                = 0x90;
    constexpr uint8_t PARAM_SW_MAJOR              = 0x91;
    constexpr uint8_t PARAM_SW_MINOR              = 0x92;
    // constexpr uint8_t PARAM_VTARGET            = 0x94;    // AVRISP_2 사용 불가
    // constexpr uint8_t PARAM_VADJUST            = 0x95;    // AVRISP_2 사용 불가
    // constexpr uint8_t PARAM_OSC_PSCALE         = 0x96;    // AVRISP_2 사용 불가
    // constexpr uint8_t PARAM_OSC_CMATCH         = 0x97;    // AVRISP_2 사용 불가
    constexpr uint8_t PARAM_SCK_DURATION          = 0x98;
    // constexpr uint8_t PARAM_TOPCARD_DETECT     = 0x9A;    // AVRISP_2 사용 불가
    constexpr uint8_t PARAM_STATUS                = 0x9C;
    // constexpr uint8_t PARAM_DATA               = 0x9D;    // AVRISP_2 사용 불가
    constexpr uint8_t PARAM_RESET_POLARITY        = 0x9E;
    constexpr uint8_t PARAM_CONTROLLER_INIT       = 0x9F;

    /**
     * @brief STK500 응답 코드
     */
    constexpr uint8_t ANSWER_CKSUM_ERROR          = 0xB0;

    /**
     * @brief STK500 In-system Programming에 사용되는 명령 코드
     */
    constexpr uint8_t CMD_ENTER_PROGMODE_ISP      = 0x10;
    constexpr uint8_t CMD_LEAVE_PROGMODE_ISP      = 0x11;
    constexpr uint8_t CMD_CHIP_ERASE_ISP          = 0x12;
    constexpr uint8_t CMD_PROGRAM_FLASH_ISP       = 0x13;
    constexpr uint8_t CMD_READ_FLASH_ISP          = 0x14;
    constexpr uint8_t CMD_PROGRAM_EEPROM_ISP      = 0x15;
    constexpr uint8_t CMD_READ_EEPROM_ISP         = 0x16;
    constexpr uint8_t CMD_PROGRAM_FUSE_ISP        = 0x17;
    constexpr uint8_t CMD_READ_FUSE_ISP           = 0x18;
    constexpr uint8_t CMD_PROGRAM_LOCK_ISP        = 0x19;
    constexpr uint8_t CMD_READ_LOCK_ISP           = 0x1A;
    constexpr uint8_t CMD_READ_SIGNATURE_ISP      = 0x1B;
    constexpr uint8_t CMD_READ_OSCCAL_ISP         = 0x1C;
    constexpr uint8_t CMD_SPI_MULTI               = 0x1D;

    /**
     * @brief STK500 Parallel Programming 모드에서 사용되는 명령 코드
     */
    constexpr uint8_t CMD_ENTER_PROGMODE_PP       = 0x20;
    constexpr uint8_t CMD_LEAVE_PROGMODE_PP       = 0x21;
    constexpr uint8_t CMD_CHIP_ERASE_PP           = 0x22;
    constexpr uint8_t CMD_PROGRAM_FLASH_PP        = 0x23;
    constexpr uint8_t CMD_READ_FLASH_PP           = 0x24;
    constexpr uint8_t CMD_PROGRAM_EEPROM_PP       = 0x25;
    constexpr uint8_t CMD_READ_EEPROM_PP          = 0x26;
    constexpr uint8_t CMD_PROGRAM_FUSE_PP         = 0x27;
    constexpr uint8_t CMD_READ_FUSE_PP            = 0x28;
    constexpr uint8_t CMD_PROGRAM_LOCK_PP         = 0x29;
    constexpr uint8_t CMD_READ_LOCK_PP            = 0x2A;
    constexpr uint8_t CMD_READ_SIGNATURE_PP       = 0x2B;
    constexpr uint8_t CMD_READ_OSCCAL_PP          = 0x2C;
    constexpr uint8_t CMD_SET_CONTROL_STACK       = 0x2D;

    /**
     * @brief STK500 High Voltage Serial Programming 모드에서 사용되는 명령 코드
     */
    constexpr uint8_t CMD_ENTER_PROGMODE_HVSP     = 0x30;
    constexpr uint8_t CMD_LEAVE_PROGMODE_HVSP     = 0x31;
    constexpr uint8_t CMD_CHIP_ERASE_HVSP         = 0x32;
    constexpr uint8_t CMD_PROGRAM_FLASH_HVSP      = 0x33;
    constexpr uint8_t CMD_READ_FLASH_HVSP         = 0x34;
    constexpr uint8_t CMD_PROGRAM_EEPROM_HVSP     = 0x35;
    constexpr uint8_t CMD_READ_EEPROM_HVSP        = 0x36;
    constexpr uint8_t CMD_PROGRAM_FUSE_HVSP       = 0x37;
    constexpr uint8_t CMD_READ_FUSE_HVSP          = 0x38;
    constexpr uint8_t CMD_PROGRAM_LOCK_HVSP       = 0x39;
    constexpr uint8_t CMD_READ_LOCK_HVSP          = 0x3A;
    constexpr uint8_t CMD_READ_SIGNATURE_HVSP     = 0x3B;
    constexpr uint8_t CMD_READ_OSCCAL_HVSP        = 0x3C;
}}