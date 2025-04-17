/**
 * @file MelsecParser.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */




#pragma once

#include <string>
#include "include/MelsecCommonHeader.h"
#include "IM/Node/Include/TypeDefinitions.h"



namespace muffin
{
    class MelsecParser
    {
    public:
        MelsecParser();
        ~MelsecParser();

    public:
        Status ParseReadResponseASCII(const uint8_t* frame, size_t length, bool isBit, uint16_t* outBuffer);
        Status ParseReadResponseBinary(const uint8_t* frame, size_t length, bool isBit, uint16_t* outBuffer);
    
    public:
        Status ParseWriteResponseASCII(const uint8_t* frame, size_t length, bool isBit);
        Status ParseWriteResponseBinary(const uint8_t* frame, size_t length, bool isBit);

    // @lsj 비어있는 접근제한자는 삭제!
    private:
    
    
    
    
    
    
    };
    
}
