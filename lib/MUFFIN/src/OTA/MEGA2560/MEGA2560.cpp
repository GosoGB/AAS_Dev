/**
 * @file MEGA2560.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ATmega2560 MCU를 업데이트 하는 클래스를 정의합니다.
 * 
 * @date 2024-11-28
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#if defined(MODLINK_T2)



#include <HardwareSerial.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/Command.h"
#include "MEGA2560.h"



namespace muffin { namespace ota {

    const char* ConvertToString(const uint8_t statusCode)
    {
        switch (statusCode)
        {
        case STATUS_CMD_OK:
            return "STATUS_CMD_OK";
        case STATUS_CMD_TOUT:
            return "STATUS_CMD_TOUT";
        case STATUS_RDY_BSY_TOUT:
            return "STATUS_RDY_BSY_TOUT";
        case STATUS_SET_PARAM_MISSING:
            return "STATUS_SET_PARAM_MISSING";
        case STATUS_CMD_FAILED:
            return "STATUS_CMD_FAILED";
        case STATUS_CKSUM_ERROR:
            return "STATUS_CKSUM_ERROR";
        case STATUS_CMD_UNKNOWN:
            return "STATUS_CMD_UNKNOWN";
        default:
            return "";
        }
    }

    /**
     * @todo 향후에 파라미터 설정 기능 구현해야 합니다.
     * @todo 설정해야 하는 파라미터가 있을지 생각해봐야 합니다.
     * 
     * @code {.cpp}
     * 
     * ret = setParameters();
     * if (ret != Status::Code::GOOD)
     * {
     *     LOG_ERROR(logger, "FAILED TO SET PROGRAMMING PARAMETERS");
     *     return ret;
     * }
     * 
     * @endcode
     */
    Status MEGA2560::Init(const size_t totalSize)
    {
        initUART();
        initGPIO();
        resetMEGA2560();

        Status ret = signOn();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SIGN ON ATmega2560 ISP");
            goto ON_FAIL;
        }

        ret = enterProgrammingMode();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO ENTER PROGRAMMING MODE");
            goto ON_FAIL;
        }

        LOG_INFO(logger, "Initialized for OTA");
        return ret;

    ON_FAIL:
        TearDown();
        return ret;
    }
    
    void MEGA2560::TearDown()
    {
        digitalWrite(RELAY_PIN, LOW);
        Serial2.end();
    }
    
    Status MEGA2560::LoadAddress(const uint32_t address)
    {
        // LOG_INFO(logger, "Loading address 0x%X", address);

        const uint32_t extendedAddress = address | (1 << 31);
        const uint8_t address01 = (extendedAddress >> 24) & 0xFF;
        const uint8_t address02 = (extendedAddress >> 16) & 0xFF;
        const uint8_t address03 = (extendedAddress >>  8) & 0xFF;
        const uint8_t address04 = extendedAddress & 0xFF;

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + 5 + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start           =  MESSAGE_START;
        command.Header.SequnceID       =  mSequenceID;
        command.Header.SizeHighByte    =  0x00;
        command.Header.SizeLowByte     =  0x05;
        command.Header.Token           =  TOKEN;
        command.MessageBody[0]         =  CMD_LOAD_ADDRESS;
        command.MessageBody[1]         =  address01;
        command.MessageBody[2]         =  address02;
        command.MessageBody[3]         =  address03;
        command.MessageBody[4]         =  address04;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(timeout_e::ALL_OTHER_COMMANDS, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO LOAD ADDRESSS 0x%X", address);
            return ret;
        }
        
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        ++mSequenceID;
        // LOG_INFO(logger, "Loaded address 0x%X", address);
        return Status(Status::Code::GOOD);
    }
    
    Status MEGA2560::ProgramFlashISP(const page_t page)
    {
        ASSERT((page.Size == PAGE_SIZE), "INVALID PAGE SIZE");
        // LOG_INFO(logger, "Programming Flash ISP");

        constexpr uint16_t bodySize  = 10 + PAGE_SIZE;

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + bodySize + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start           =  MESSAGE_START;
        command.Header.SequnceID       =  mSequenceID;
        command.Header.SizeHighByte    =  (bodySize >> 8) & 0xFF;
        command.Header.SizeLowByte     =  bodySize & 0xFF;
        command.Header.Token           =  TOKEN;
        command.MessageBody[0]         =  CMD_PROGRAM_FLASH_ISP;        // Command ID
        command.MessageBody[1]         =  (PAGE_SIZE >> 8) & 0xFF;      // NumBytes High
        command.MessageBody[2]         =  PAGE_SIZE & 0xFF;             // NumBytes Low
        command.MessageBody[3]         =  0xC1;                         // Mode
        command.MessageBody[4]         =    10;                         // Delay in milliseconds
        command.MessageBody[5]         =  0x40;                         // Command 1
        command.MessageBody[6]         =  0x4C;                         // Command 2
        command.MessageBody[7]         =  0x00;                         // Command 3
        command.MessageBody[8]         =  0x00;                         // Poll 1
        command.MessageBody[9]         =  0x00;                         // Poll 2
        for (uint16_t i = 0; i < PAGE_SIZE; ++i)                        // Data
        {
            command.MessageBody[10 + i] = page.Data[i];
        }
        calculateChecksum(&command);
        sendCommand(command);
    
        msg_t response;
        Status ret = receiveResponse(timeout_e::CMD_PROGRAM_FLASH_ISP, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PROGRAM FLASH ISP: %s", ret.c_str());
            return ret;
        }
        
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));
        
        ++mSequenceID;
        // LOG_INFO(logger, "Programmed Flash ISP");
        return Status(Status::Code::GOOD);
    }

    Status MEGA2560::ReadFlashISP(const uint16_t readLength, page_t* outputPage)
    {
        ASSERT((outputPage != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");
        // LOG_INFO(logger, "Reading Flash ISP: %u", readLength);

        const uint8_t lengthByteHIGH = (readLength >>  8) & 0xFF;
        const uint8_t lengthByteLOW  = readLength & 0xFF;
        const uint8_t byteSelection  = 0x01 < 2;

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + 4 + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start           =  MESSAGE_START;
        command.Header.SequnceID       =  mSequenceID;
        command.Header.SizeHighByte    =  0x00;
        command.Header.SizeLowByte     =  0x04;
        command.Header.Token           =  TOKEN;
        command.MessageBody[0]         =  CMD_READ_FLASH_ISP;
        command.MessageBody[1]         =  lengthByteHIGH;
        command.MessageBody[2]         =  lengthByteLOW;
        command.MessageBody[3]         =  byteSelection;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(timeout_e::CMD_READ_FLASH_ISP, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ FROM FLASH ISP: %s", ret.c_str());
            return ret;
        }
        
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        const uint8_t ANSWER_FORMAT_LENGTH = 3;
        if (response.Size != (ANSWER_FORMAT_LENGTH + readLength + MESSAGE_OVERHEAD + MESSAGE_CHECKSUM))
        {
            LOG_ERROR(logger, "THE LENGTH OF BYTES READ: 0x%X != 0x%X", response.Size, (ANSWER_FORMAT_LENGTH + readLength + MESSAGE_OVERHEAD + MESSAGE_CHECKSUM));
            return Status(Status::Code::BAD_DATA_LOST);
        }

        outputPage->Size = 0;
        memset(outputPage->Data, 0, PAGE_SIZE);

        for (uint16_t i = 0; i < readLength; ++i)
        {
            outputPage->Data[i] = response.MessageBody[2+i];
            ++outputPage->Size;
        }

        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        ++mSequenceID;
        // LOG_INFO(logger, "Finished reading Flash ISP: %u", readLength);
        return Status(Status::Code::GOOD);
    }

    Status MEGA2560::LeaveProgrammingMode()
    {
        LOG_INFO(logger, "Leaving programming mode ISP");

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + 3 + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start           =   MESSAGE_START;
        command.Header.SequnceID       =   mSequenceID;
        command.Header.SizeHighByte    =   0x00;
        command.Header.SizeLowByte     =   0x03;
        command.Header.Token           =   TOKEN;
        command.MessageBody[0]         =   CMD_LEAVE_PROGMODE_ISP;   // Command ID
        command.MessageBody[1]         =   1;                        // PreDelay
        command.MessageBody[2]         =   1;                        // PostDelay
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(timeout_e::ALL_OTHER_COMMANDS, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO LEAVE PROGRAMMING MODE ISP");
            return ret;
        }
        
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    void MEGA2560::initUART()
    {
        Serial2.end();
        /**
         * @todo ATmega2560 업데이트 전에 다른 태스크가 포트를 사용하지 않도록 확인해야 합니다.
         *       예: uxTaskGetNumberOfTasks() 함수로 현재 실행 중인 태스크 개수 확인
         */
        ASSERT((Serial2 == false), "SERIAL PORT MUST BE ENDED BEFORE UPDATE");

        /**
         * @brief STK500 프로토콜은 115,200bps, 8N1 설정으로 통신을 수행합니다. 
         * @ref "AVR068: STK500 Communication Protocol" 문서 참조
         */
        Serial2.begin(115200, SERIAL_8N1);
    }

    void MEGA2560::initGPIO()
    {
        pinMode(RESET_PIN, OUTPUT);
        pinMode(RELAY_PIN, OUTPUT);

        /**
         * @brief ESP32가 ATmega2560 칩셋을 리셋시킬 때 사용
         * @li HIGH: ATmega2560 칩셋 리셋 (X)
         * @li LOW:  ATmega2560 칩셋 리셋 (O)
         */
        digitalWrite(RESET_PIN, HIGH);
        
        /**
         * @brief ESP32 시리얼 포트 2번을 ATmega2560 시리얼 포트 0번에 연결할 때 사용
         * @li HIGH: [ESP]시리얼 포트 2번 -- [MEGA]시리얼 포트 0번 연결 (O)
         * @li LOW:  [ESP]시리얼 포트 2번 -- [MEGA]시리얼 포트 0번 연결 (X)
         */
        digitalWrite(RELAY_PIN, HIGH);
    }

    void MEGA2560::resetMEGA2560()
    {
        LOG_INFO(logger, "Start to reset ATmega2560");

        digitalWrite(RESET_PIN, LOW);
        vTaskDelay(1 / portTICK_PERIOD_MS);

        digitalWrite(RESET_PIN, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        digitalWrite(RESET_PIN, LOW);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        
        digitalWrite(RESET_PIN, HIGH);
        vTaskDelay(800 / portTICK_PERIOD_MS);

    /**
     * @todo PoC 때는 아래 코드를 넣었으나 현재는 필요 없을 수 있습니다. 검증 후 삭제해야 합니다.
     * @code {.cpp}
     * while (Serial2.available())
     * {
     *     Serial2.read();
     * }
     * @endcode
     */   
        LOG_INFO(logger, "Finished resetting the ATmega2560");
    }

    Status MEGA2560::signOn()
    {
        LOG_INFO(logger, "Start signing on");

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + 1 + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start        = MESSAGE_START;
        command.Header.SequnceID    = mSequenceID;
        command.Header.SizeHighByte = 0x00;
        command.Header.SizeLowByte  = 0x01;
        command.Header.Token        = TOKEN;
        command.MessageBody[0]      = CMD_SIGN_ON;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(timeout_e::CMD_SIGN_ON, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SIGN ON TO THE ISP");
            return ret;
        }
        
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        const uint8_t length   = response.MessageBody[2];
        char signature[length + 1] = { 0 };
        memcpy(signature, response.MessageBody + 3, length);

        if (strcmp(signature, "AVRISP_2") != 0)
        {
            LOG_ERROR(logger, "UNKNOWN SIGNATURE STRING: %s", signature);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        
        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Signature Length: 0x%X", response.MessageBody[2]);
        // LOG_DEBUG(logger, "Signature: %s", signature);
        LOG_INFO(logger, "Sign-on to the programmer of ATmega2560 MCU");

        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    Status MEGA2560::setParameters()
    {
        LOG_INFO(logger, "Start setting programming parameters");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status MEGA2560::enterProgrammingMode()
    {
        LOG_INFO(logger, "Entering programming mode ISP");

        msg_t command;
        command.Size = MESSAGE_OVERHEAD + 8 + MESSAGE_CHECKSUM;
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND SIZE CANNOT EXCEED MAX MESSAGE SIZE");
        command.Header.Start           =   MESSAGE_START;
        command.Header.SequnceID       =   mSequenceID;
        command.Header.SizeHighByte    =   0x00;
        command.Header.SizeLowByte     =   0x08;
        command.Header.Token           =   TOKEN;
        command.MessageBody[0]         =   CMD_ENTER_PROGMODE_ISP;   // Command ID
        command.MessageBody[1]         =   200;                      // timeout
        command.MessageBody[2]         =   100;                      // stabDelay
        command.MessageBody[3]         =    25;                      // cmdexeDelay
        command.MessageBody[4]         =    32;                      // synchLoops
        command.MessageBody[5]         =     0;                      // byteDelay
        command.MessageBody[6]         =  0x53;                      // pollValue
        command.MessageBody[7]         =     3;                      // pollIndex
        // command.MessageBody[8]         =     0;                      // cmd1
        // command.MessageBody[9]         =     0;                      // cmd2
        // command.MessageBody[10]        =     0;                      // cmd3
        // command.MessageBody[11]        =     0;                      // cmd4
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(timeout_e::ALL_OTHER_COMMANDS, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO ENTER PROGRAMMING MODE ISP");
            return ret;
        }
        
        LOG_DEBUG(logger, "Target: %02X, Actual: %02X", command.Header.SequnceID, response.Header.SequnceID);
        if (command.Header.SequnceID != response.Header.SequnceID)
        {
            LOG_ERROR(logger, "SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        
        if (command.MessageBody[0] != response.MessageBody[0])
        {
            LOG_ERROR(logger, "ANSWER ID DOES NOT MATCH WITH COMMAND ID");
            return Status(Status::Code::BAD_IDENTITY_TOKEN_INVALID);
        }

        if (STATUS_CMD_OK != response.MessageBody[1])
        {
            LOG_ERROR(logger, "FAILED WITH STATUS CODE: 0x%X", response.MessageBody[1]);
            return Status(Status::Code::BAD);
        }

        // LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        // LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        // LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));
        
        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    void MEGA2560::calculateChecksum(msg_t* outputMessage)
    {
        ASSERT((outputMessage != nullptr), "OUTPUT PARAMETER \"MESSAGE\" CANNOT BE A NULL POINTER");
        ASSERT((outputMessage->Size <= MAX_MESSAGE_SIZE), "COMMAND LENGTH CANNOT EXCEED THE MAXIMUM MESSAGE SIZE");

        const uint16_t sizeHigh = static_cast<uint16_t>(outputMessage->Header.SizeHighByte) << 8;
        const uint8_t  sizeLow  = outputMessage->Header.SizeLowByte;
        const uint16_t size = sizeHigh | sizeLow;
        // LOG_DEBUG(logger, "Message body size: %u", size);

        outputMessage->Checksum = 0;
        outputMessage->Checksum ^= outputMessage->Header.Start;
        outputMessage->Checksum ^= outputMessage->Header.SequnceID;
        outputMessage->Checksum ^= outputMessage->Header.SizeHighByte;
        outputMessage->Checksum ^= outputMessage->Header.SizeLowByte;
        outputMessage->Checksum ^= outputMessage->Header.Token;
        for (uint16_t i = 0; i < size; ++i)
        {
            outputMessage->Checksum ^= outputMessage->MessageBody[i];
        }

        // LOG_DEBUG(logger, "Checksum: 0x%X", outputMessage->Checksum);
    }

    void MEGA2560::calculateChecksum(std::vector<uint8_t>& vector, uint8_t* outputChecksum)
    {
        ASSERT((vector.size() > (MESSAGE_OVERHEAD + MESSAGE_CHECKSUM)), "INVALID LENGTH FOR A RESPONSE");
        ASSERT((outputChecksum != nullptr), "OUTPUT PARAMETER \"CHECKSUM\" CANNOT BE A NULL POINTER");
        
        *outputChecksum = 0;
        for (uint16_t i = 0; i < (vector.size() - 1); ++i)
        {
            *outputChecksum ^= vector[i];
        }
    }

    int MEGA2560::sendCommand(const msg_t command)
    {
        ASSERT((command.Size <= MAX_MESSAGE_SIZE), "COMMAND LENGTH CANNOT EXCEED THE MAXIMUM MESSAGE SIZE");

        const uint16_t bodySizeHigh = static_cast<uint16_t>(command.Header.SizeHighByte) << 8;
        const uint8_t  bodySizeLow  = command.Header.SizeLowByte;
        const uint16_t bodySize     = bodySizeHigh | bodySizeLow;
        const uint16_t size         = MESSAGE_OVERHEAD + bodySize + MESSAGE_CHECKSUM;
        // LOG_DEBUG(logger, "Message size: %u", size);
        ASSERT((bodySize > 0), "MESSAGE BODY CANNOT BE EMPTY");

        uint8_t message[size] = { 0 };
        message[0] = command.Header.Start;
        message[1] = command.Header.SequnceID;
        message[2] = command.Header.SizeHighByte;
        message[3] = command.Header.SizeLowByte;
        message[4] = command.Header.Token;
        for (uint16_t i = 5; i < (5 + bodySize); ++i)
        {
            message[i] = command.MessageBody[(i - 5)];
        }
        message[(size - 1)] = command.Checksum;

    // #if defined(DEBUG)
    //     Serial.print("Command: ");
    //     for (size_t i = 0; i < size; ++i)
    //     {
    //         Serial.printf("0x%X ", message[i]);
    //     }
    //     Serial.println();
    // #endif
        return Serial2.write(message, size);
    }

    Status MEGA2560::receiveResponse(const timeout_e timeout, msg_t* outputResponse)
    {
        ASSERT((outputResponse != nullptr), "OUTPUT PARAMETER \"RESPONSE\" CANNOT BE A NULL POINTER");

        const uint32_t startMillis = millis();
        std::vector<uint8_t> rxd;
        rxd.reserve(32);

        while (uint32_t(millis() - startMillis) < static_cast<uint16_t>(timeout))
        {
            while (Serial2.available())
            {
                rxd.emplace_back(Serial2.read());
            }

            if (rxd.size() < (MESSAGE_OVERHEAD + MESSAGE_CHECKSUM))
            {
                continue;
            }
            
            const uint8_t lastByte = rxd.back();
            uint8_t calculatedChecksum = 0;
            calculateChecksum(rxd, &calculatedChecksum);
            if (lastByte == calculatedChecksum)
            {
                goto ON_RECEIVE;
            }
        }
        return Status(Status::Code::BAD_TIMEOUT);

    ON_RECEIVE:
        if (rxd[0] != MESSAGE_START)
        {
            LOG_ERROR(logger, "INVALID FORMAT: MESSAGE_START BYTE NOT FOUND");
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        if (rxd[1] != mSequenceID)
        {
            LOG_ERROR(logger, "INVALID ID: SEQUENCE ID DOES NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }

        if (rxd[4] != TOKEN)
        {
            LOG_ERROR(logger, "INVALID FORMAT: TOKEN BYTE NOT FOUND");
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        
        uint16_t idx = 0;
        outputResponse->Header.Start          = rxd[idx++];
        outputResponse->Header.SequnceID      = rxd[idx++];
        outputResponse->Header.SizeHighByte   = rxd[idx++];
        outputResponse->Header.SizeLowByte    = rxd[idx++];
        outputResponse->Header.Token          = rxd[idx++];
        for (; idx < (rxd.size() - 1); ++idx)
        {
            outputResponse->MessageBody[(idx - 5)] = rxd[idx];
        }
        outputResponse->Checksum = rxd.back();
        ++idx;
        ASSERT((idx <= MAX_MESSAGE_SIZE), "RESPONSE LENGTH CANNOT EXCEED THE MAXIMUM MESSAGE SIZE");
        outputResponse->Size = idx;
        return Status(Status::Code::GOOD);
    }
}}



#endif