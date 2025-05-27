/**
 * @file Microchip24AA02E.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Microchip24AA02E 칩에 저장된 MAC 주소를 읽는 클래스를 선언합니다.
 * 
 * @date 2025-04-28
 * @version 1.0.0
 * 
 * @todo External LINK의 디바이스 주소를 정의해야 합니다. 
 * 
 * @copyright Copyright (c) 2025
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin {


    class Microchip24AA02E
    {
    public:
        bool Read(const bool isLINK, uint8_t mac[]);
    private:
        int8_t readRegister(const uint8_t offset, const bool isLINK);
        bool verifyMacAddress(const uint8_t mac[]);
    private:
        static const uint8_t mSDA = 2;
        static const uint8_t mSCL = 1;
        static const uint8_t mDeviceAddressBOARD  = 0x50;
        static const uint8_t mDeviceAddressLINK   = 0x00;
    };
}