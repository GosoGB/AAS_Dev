/**
 * @file MEGA2560.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ATmega2560 MCU를 업데이트 하는 클래스를 선언합니다.
 * 
 * @date 2024-11-28
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#if defined(MT10)



#pragma once

#include <vector>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"



namespace muffin { namespace ota {

    class MEGA2560
    {
    public:
        MEGA2560() {}
        virtual ~MEGA2560() {}
    public:
        Status Init(const size_t totalSize);
        void TearDown();
    public:
        Status LoadAddress(const uint32_t address);
        Status ProgramFlashISP(const page_t page);
        Status ReadFlashISP(const uint16_t readLength, page_t* outputPage);
        Status LeaveProgrammingMode();
    private:
        void initUART();
        void initGPIO();
        void resetMEGA2560();
    private:
        Status signOn();
        Status setParameters();
        Status enterProgrammingMode();
    private:
        int sendCommand(const msg_t command);
        Status receiveResponse(const timeout_e timeout, msg_t* outputResponse);
        void calculateChecksum(msg_t* outputMessage);
        void calculateChecksum(std::vector<uint8_t>& vector, uint8_t* outputChecksum);
    private:
        uint8_t mSequenceID = 0;
    private:
        const uint8_t RELAY_PIN = 12;
        const uint8_t RESET_PIN = 13;
        const uint8_t MESSAGE_OVERHEAD = 5;
        const uint8_t MESSAGE_CHECKSUM = 1;
    };
}}



#endif