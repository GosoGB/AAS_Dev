/**
 * @file MEGA2560.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ATmega2560 MCU를 업데이트 하는 클래스를 선언합니다.
 * 
 * @date 2024-11-15
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"



namespace muffin { namespace ota {

    class MEGA2560
    {
    public:
        MEGA2560();
        virtual ~MEGA2560();
    public:
        Status Init();
        void TearDown();
    public:
        Status LoadAddress(const uint32_t address);
        Status ProgramFlashISP();
        Status ReadFlashISP(const uint16_t readLength);
        Status LeaveProgrammingMode();
        // Status Parse();
        // Status Write();
        // Status Read();
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
        Status receiveResponse(const uint16_t timeout, msg_t* outputResponse);
        void calculateChecksum(msg_t* outputMessage);
        void calculateChecksum(std::vector<uint8_t>& vector, uint8_t* outputChecksum);
    private:
        uint8_t mSequenceID = 0;
    private:
        const uint8_t RELAY_PIN = 12;
        const uint8_t RESET_PIN = 13;
        const uint8_t MESSAGE_OVERHEAD = 5;
        const uint8_t MESSAGE_CHECKSUM = 1;
        const uint8_t MIN_DELAY_IN_MILLIS = 2;
        const uint8_t MAX_DELAY_IN_MILLIS = 100;
        const uint16_t BLOCK_SIZE = 256;
        const uint16_t PAGE_SIZE_MAX = 20 * 1024;
        uint8_t* mPage = nullptr;
        int32_t mBlockCount = 0;
        size_t mByteCount = 0;
    };
}}