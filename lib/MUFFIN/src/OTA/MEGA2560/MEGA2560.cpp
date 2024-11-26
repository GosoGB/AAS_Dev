/**
 * @file MEGA2560.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ATmega2560 MCU를 업데이트 하는 클래스를 정의합니다.
 * 
 * @date 2024-11-15
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <HardwareSerial.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/Command.h"
#include "Include/HexParser.h"
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

    MEGA2560::MEGA2560()
    {
    }
    
    MEGA2560::~MEGA2560()
    {
    }

    Status MEGA2560::Init()
    {
        mPage = new(std::nothrow) uint8_t[PAGE_SIZE_MAX];
        if (mPage == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR OTA ATmega2560");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        HexParser parser;
        parser.Parse("/ota_test.hex", mPage, &mBlockCount, &mByteCount);
        LOG_DEBUG(logger, "Block Count: %u", mBlockCount);
        LOG_DEBUG(logger, "Byte Count: %u", mByteCount);

        initUART();
        initGPIO();
        resetMEGA2560();

        Status ret = signOn();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO GET SYNC WITH MEGA2560");
            return ret;
        }
        
        // ret = setParameters();
        // if (ret != Status::Code::GOOD)
        // {
        //     LOG_ERROR(logger, "FAILED TO SET PROGRAMMING PARAMETERS");
        //     return ret;
        // }

        ret = enterProgrammingMode();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO ENTER PROGRAMMING MODE");
            return ret;
        }

        return ret;
    }
    
    void MEGA2560::TearDown()
    {
        digitalWrite(RELAY_PIN, LOW);
        Serial2.end();
    }

    Status MEGA2560::LoadAddress(const uint32_t address)
    {
        LOG_INFO(logger, "Loading address 0x%X", address);

        const uint32_t extendedAddress = address | (1 << 31);
        const uint8_t address01 = (extendedAddress >> 24) & 0xFF;
        const uint8_t address02 = (extendedAddress >> 16) & 0xFF;
        const uint8_t address03 = (extendedAddress >>  8) & 0xFF;
        const uint8_t address04 = extendedAddress & 0xFF;

        msg_t command;
        command.Header.Start           =  MESSAGE_START;
        command.Header.SequnceID       =  mSequenceID;
        command.Header.SizeHighByte    =  0x00;
        command.Header.SizeLowByte     =  0x05;
        command.Header.Token           =  TOKEN;
        command.MessageBody[0]         =  CMD_LOAD_ADDRESS; // Command ID
        command.MessageBody[1]         =  address01;
        command.MessageBody[2]         =  address02;
        command.MessageBody[3]         =  address03;
        command.MessageBody[4]         =  address04;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(1000, &response);
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

        LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }
    
    Status MEGA2560::ProgramFlashISP()
    {
        LOG_INFO(logger, "Writing Flash ISP");

        size_t writtenBytes = 0;

        while (true)
        {
            const uint16_t remainedBytes = mByteCount - writtenBytes;
            const uint16_t bytesToWrite  = remainedBytes < BLOCK_SIZE ? 
                remainedBytes : 
                BLOCK_SIZE;
            uint16_t commandSize = 10 + bytesToWrite;

            msg_t command;
            command.Header.Start           =  MESSAGE_START;
            command.Header.SequnceID       =  mSequenceID;
            command.Header.SizeHighByte    =  (commandSize >> 8) & 0xFF;
            command.Header.SizeLowByte     =  commandSize & 0xFF;
            command.Header.Token           =  TOKEN;
            command.MessageBody[0]         =  CMD_PROGRAM_FLASH_ISP;        // Command ID
            command.MessageBody[1]         =  (bytesToWrite >> 8) & 0xFF;   // NumBytes High
            command.MessageBody[2]         =  bytesToWrite & 0xFF;          // NumBytes Low
            command.MessageBody[3]         =  0xC1;                         // mode
            command.MessageBody[4]         =    10;                         // delay in milliseconds
            command.MessageBody[5]         =  0x40;                         // cmd1
            command.MessageBody[6]         =  0x4C;                         // cmd2
            command.MessageBody[7]         =  0x00;                         // cmd3
            command.MessageBody[8]         =  0x00;                         // poll1
            command.MessageBody[9]         =  0x00;                         // poll2
            for (uint16_t i = 0; i < bytesToWrite; ++i)                     // data
            {
                command.MessageBody[10 + i] = mPage[writtenBytes++];
            }
        
            calculateChecksum(&command);
            sendCommand(command);
            msg_t response;
            Status ret = receiveResponse(1000, &response);
            if (ret != Status::Code::GOOD)
            {
                // LOG_ERROR(logger, "FAILED TO READ FROM FLASH ISP: %u", readLength);
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

            LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
            LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
            LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

            ++mSequenceID;
            --mBlockCount;
        }
        return Status(Status::Code::GOOD);
    }

    Status MEGA2560::ReadFlashISP(const uint16_t readLength)
    {
        LOG_INFO(logger, "Reading Flash ISP: %u", readLength);

        const uint8_t lengthByteHIGH = (readLength >>  8) & 0xFF;
        const uint8_t lengthByteLOW  = readLength & 0xFF;
        const uint8_t byteSelection  = 0x01 < 2;

        msg_t command;
        command.Header.Start           =  MESSAGE_START;
        command.Header.SequnceID       =  mSequenceID;
        command.Header.SizeHighByte    =  0x00;
        command.Header.SizeLowByte     =  0x04;
        command.Header.Token           =  TOKEN;
        command.MessageBody[0]         =  CMD_READ_FLASH_ISP; // Command ID
        command.MessageBody[1]         =  lengthByteHIGH;
        command.MessageBody[2]         =  lengthByteLOW;
        command.MessageBody[3]         =  byteSelection;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(1000, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ FROM FLASH ISP: %u", readLength);
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

        LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        for (uint16_t i = 0; i < readLength; ++i)
        {
            Serial.printf("0x%02X,", response.MessageBody[2+i]);
        }
        Serial.println();

        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    Status MEGA2560::LeaveProgrammingMode()
    {
        LOG_INFO(logger, "Leaving programming mode ISP");

        msg_t command;
        command.Header.Start           =   MESSAGE_START;
        command.Header.SequnceID       =   mSequenceID;
        command.Header.SizeHighByte    =   0x00;
        command.Header.SizeLowByte     =   0x03;
        command.Header.Token           =   TOKEN;
        command.MessageBody[0]         =   CMD_LEAVE_PROGMODE_ISP;   // Command ID
        command.MessageBody[1]         =   1;                        // preDelay
        command.MessageBody[2]         =   1;                        // postDelay
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(1000, &response);
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

        LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));

        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    void MEGA2560::initUART()
    {
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

        while (Serial2.available())
        {
            Serial2.read();
        }
        

        LOG_INFO(logger, "Reset the ATmega2560");
    }

    /**
     * @note CMD_SIGN_ON 명령의 타임아웃은 200ms입니다.
     * @note 다음 명령의 타임아웃은 5,000ms입니다.
     * @li CMD_READ_FLASH_ISP
     * @li CMD_PROGRAM_FLASH_ISP
     * @li CMD_PROGRAM_EEPROM_ISP
     * @li CMD_READ_EEPROM_ISP
     * @note 이외의 모든 명령의 타임아웃은 1,000ms입니다.
     */
    Status MEGA2560::signOn()
    {
        LOG_INFO(logger, "Start signing on");

        msg_t command;
        command.Header.Start        = MESSAGE_START;
        command.Header.SequnceID    = mSequenceID;
        command.Header.SizeHighByte = 0x00;
        command.Header.SizeLowByte  = 0x01;
        command.Header.Token        = TOKEN;
        command.MessageBody[0]      = CMD_SIGN_ON;
        calculateChecksum(&command);
        sendCommand(command);

        msg_t response;
        Status ret = receiveResponse(200, &response);
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
        
        LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        LOG_DEBUG(logger, "Status: 0x%X", response.MessageBody[1]);
        LOG_DEBUG(logger, "Signature Length: 0x%X", response.MessageBody[2]);
        LOG_DEBUG(logger, "Signature: %s", signature);
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
        Status ret = receiveResponse(1000, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO ENTER PROGRAMMING MODE ISP");
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

        LOG_DEBUG(logger, "Answer ID: 0x%X", response.MessageBody[0]);
        LOG_DEBUG(logger, "Status Code: 0x%X", response.MessageBody[1]);
        LOG_DEBUG(logger, "Status String: %s", ConvertToString(response.MessageBody[1]));
        
        ++mSequenceID;
        return Status(Status::Code::GOOD);
    }

    void MEGA2560::calculateChecksum(msg_t* outputMessage)
    {
        ASSERT((outputMessage != nullptr), "OUTPUT PARAMETER \"MESSAGE\" CANNOT BE A NULL POINTER");

        const uint16_t sizeHigh = static_cast<uint16_t>(outputMessage->Header.SizeHighByte) << 8;
        const uint8_t  sizeLow  = outputMessage->Header.SizeLowByte;
        const uint16_t size = sizeHigh | sizeLow;
        LOG_DEBUG(logger, "Message body size: %u", size);

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

        LOG_DEBUG(logger, "Checksum: 0x%X", outputMessage->Checksum);
    }

    void MEGA2560::calculateChecksum(std::vector<uint8_t>& vector, uint8_t* outputChecksum)
    {
        ASSERT((outputChecksum  != nullptr), "OUTPUT PARAMETER \"CHECKSUM\" CANNOT BE A NULL POINTER");
        
        *outputChecksum = 0;

        for (uint16_t i = 0; i < (vector.size() - 1); ++i)
        {
            *outputChecksum ^= vector[i];
        }
    }

    int MEGA2560::sendCommand(const msg_t command)
    {
        const uint16_t bodySizeHigh = static_cast<uint16_t>(command.Header.SizeHighByte) << 8;
        const uint8_t  bodySizeLow  = command.Header.SizeLowByte;
        const uint16_t bodySize     = bodySizeHigh | bodySizeLow;
        ASSERT((bodySize > 0), "MESSAGE BODY CANNOT BE EMPTY");
        const uint16_t size         = MESSAGE_OVERHEAD + bodySize + MESSAGE_CHECKSUM;
        LOG_DEBUG(logger, "Message size: %u", size);

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

        Serial.print("Command: ");
        for (size_t i = 0; i < size; ++i)
        {
            Serial.printf("0x%X ", message[i]);
        }
        Serial.println();
        return Serial2.write(message, size);
    }

    Status MEGA2560::receiveResponse(const uint16_t timeout, msg_t* outputResponse)
    {
        ASSERT((outputResponse != nullptr), "OUTPUT PARAMETER \"RESPONSE\" CANNOT BE A NULL POINTER");

        const uint32_t startMillis = millis();
        std::vector<uint8_t> rxd;
        rxd.reserve(32);

        while (uint32_t(millis() - startMillis) < timeout)
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
        
        outputResponse->Header.Start          = rxd[0];
        outputResponse->Header.SequnceID      = rxd[1];
        outputResponse->Header.SizeHighByte   = rxd[2];
        outputResponse->Header.SizeLowByte    = rxd[3];
        outputResponse->Header.Token          = rxd[4];
        for (uint16_t i = 5; i < (rxd.size() - 1); ++i)
        {
            outputResponse->MessageBody[(i - 5)] = rxd[i];
        }
        outputResponse->Checksum = rxd.back();
        return Status(Status::Code::GOOD);
    }
}}