/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief STK500 Ver.2 프로토콜에서 사용하는 메시지 형식을 정의합니다.
 * @ref https://github.com/arduino/Arduino-stk500v2-bootloader
 * 
 * @date 2024-11-08
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>

#include "Command.h"



namespace muffin { namespace ota {

    typedef struct STK500MessageHeader
    {
        uint8_t Start;
        uint8_t SequnceID;
        uint8_t SizeHighByte;
        uint8_t SizeLowByte;
        uint8_t Token;
    } header_t;

    typedef struct STK500MessageType
    {
        header_t Header;
        uint8_t MessageBody[275];
        uint8_t Checksum;
    } msg_t;
}}