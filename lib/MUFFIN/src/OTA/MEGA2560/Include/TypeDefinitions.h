/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief STK500 Ver.2 프로토콜에서 사용하는 메시지 형식을 정의합니다.
 * @ref https://github.com/arduino/Arduino-stk500v2-bootloader
 * 
 * @date 2024-11-28
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#if defined(MODLINK_T2)



#pragma once

#include <sys/_stdint.h>

#include "Command.h"



namespace muffin { namespace ota {

    static constexpr uint16_t MAX_MESSAGE_SIZE = 275;
    static constexpr uint16_t PAGE_SIZE = 256;
    
    typedef struct STK500V2MessageHeader
    {
        uint8_t Start;
        uint8_t SequnceID;
        uint8_t SizeHighByte;
        uint8_t SizeLowByte;
        uint8_t Token;
    } header_t;

    typedef struct STK500V2MessageType
    {
        header_t Header;
        uint8_t MessageBody[MAX_MESSAGE_SIZE];
        uint8_t Checksum;
        uint16_t Size;
    } msg_t;

    typedef struct STK500V2IntelHexPageType
    {
        uint8_t Data[PAGE_SIZE];
        uint16_t Size;
    } page_t;

    typedef enum class STK500V2TimeoutEnum
        : uint16_t
    {
        CMD_SIGN_ON             = 200,
        CMD_READ_FLASH_ISP      = 5000,
        CMD_PROGRAM_FLASH_ISP   = 5000,
        CMD_PROGRAM_EEPROM_ISP  = 5000,
        CMD_READ_EEPROM_ISP     = 5000,
        ALL_OTHER_COMMANDS      = 1000,
    } timeout_e;
}}



#endif