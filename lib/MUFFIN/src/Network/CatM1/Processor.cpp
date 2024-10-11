/**
 * @file Processor.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈과의 모든 통신을 처리하는 클래스를 선언합니다.
 * 
 * @date 2024-09-08
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Logger/Logger.h"
#include "Processor.h"



namespace muffin {

    /**
     * @brief Received when the mobile equipment(ME) is initialized.
     * @details The URC code "RDY" indicates the modem's baseband 
     * processor(BBP) has finished booting up and is ready to establish 
     * communication with the network.
     * @note The variables starts with URC codes, which stands for
     * unsolicited response code in the AT command context to inform
     * notifications to the device asynchronously.
     */
    const std::string urcRDY("\r\nRDY\r\n");

    /**
     * @brief URC code indicates all function of the ME is initialized.
     * @details Check if all of the ME functionality is available after urcRDY. 
     * The parameters for the AT+CFUN commands are listed below but when used 
     * as URC, the parameter is fixed to "1"
     * 
     *   - 0: minimum functionality 
     *   - 1: full functionality (default)
     *   - 4: tx and rx of the ME is disabled, a.k.a. flight mode
     */
    const std::string urcCFUN("\r\n+CFUN: 1\r\n");

    /**
     * @brief urc code indicates the PIN state of the terminal adapter(TA).
     * @details check the state is "READY" to activate USIM after urcRDY.
     * the list below shows the alphanumeric string indicating whether 
     * or not some password is required.
     * 
     *   - "READY": mobile terminal(MT) is not pending for any password 
     *   - "SIM PIN": MT is waiting for (U)SIM PIN to be given 
     *   - "SIM PUK": MT is waiting for (U)SIM PUK to be given
     */
    const std::string urcCPIN("\r\n+CPIN: ");

    /**
     * @brief urc code indicates the SMS functionality is initialized.
     * @details check the SMS functionality of the ME after urcRDY.
     */
    const std::string urcQIND("\r\n+QIND: SMS DONE\r\n");

    /**
     * @brief urc code indicates the LTE module has finished booting up.
     * @details check the urcAPP of the ME after urcRDY. "APP RDY" is an 
     * abbreviation for "Application Ready.". In the context of Quectel 
     * modems like the BG96, "APP RDY" typically indicates that the modem's 
     * application processor (AP) has finished booting up and is ready to 
     * execute user applications or firmware.
     */
    const std::string urcAPPRDY("\r\nAPP RDY\r\n");

    /**
     * @brief urc code indicates received a message from the mqtt topic.
     * @details parse both the topic and the json payload from the message 
     * forwarded by the mqtt message broker. the list below shows the urc 
     * parameters and their definitions.
     * 
     *      - <socket>      MQTT socket identifier. The range is 0-5.
     *      - <msgID>       The message identifier of packet.
     *      - <topic>       The topic that received from MQTT server.
     *      - <payload>     The payload that relates to the topic name.
     */
    const std::string urcQMTRECV("+QMTRECV: ");
    const std::string urcQMTSTAT("+QMTSTAT");
    const std::string errorCodeCME("+CME ERROR: ");
    const std::string errorCode("\r\r\nERROR\r\n");


    Processor::Processor()
        : mSerial(HardwareSerial(1))
        , mRxBufferSize(1024)
        , mRxBuffer(mRxBufferSize)
        , mTimeoutMillis(50)
        , mBaudRate(baudrate_e::BDR_115200)
        , xHandle(NULL)
        , xSemaphore(NULL)
        , mTaskInterval(50)
    {
        mInitFlags.reset();
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Processor::~Processor()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Status Processor::Init()
    {
        if (mInitFlags.all() == true)
        {
            LOG_WARNING(logger, "REINITIALIZATION IS NOT ALLOWED");
            return Status(Status::Code::GOOD);
        }

        if (mInitFlags.test(init_flags_e::SERIAL_PORT_INITIALIZED) == false)
        {
            mSerial.setTimeout(mTimeoutMillis);
            if (mSerial.setRxBufferSize(mRxBufferSize) != mRxBufferSize)
            {
                LOG_ERROR(logger, "FAILED TO SET RxD BUFFER SIZE DUE TO OUT OF MEMEORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            mSerial.begin(static_cast<uint32_t>(mBaudRate), SERIAL_8N1, mPinRxD, mPinTxD);
            mInitFlags.set(init_flags_e::SERIAL_PORT_INITIALIZED);
            LOG_INFO(logger, "Initialized serial port for LTE Cat.M1 module");
        }

        if (mInitFlags.test(init_flags_e::TASK_SEMAPHORE_CREATED) == false)
        {
            xSemaphore = xSemaphoreCreateMutex();
            if (xSemaphore == NULL)
            {
                LOG_ERROR(logger, "FAILED TO CREATE SEMAPHORE");
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
            mInitFlags.set(init_flags_e::TASK_SEMAPHORE_CREATED);
            LOG_INFO(logger, "Created task semaphore");
        }
    
        if (mInitFlags.test(init_flags_e::PROCESSOR_TASK_CREATED) == false)
        {
            BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
                wrapUrcHandleTask, 
                "UrcHandleTask", 
                4096, 
                this, 
                0, 
                &xHandle, 
                0
            );

            switch (taskCreationResult)
            {
            case pdPASS:
                mInitFlags.set(init_flags_e::PROCESSOR_TASK_CREATED);
                break;
            case pdFAIL:
                LOG_ERROR(logger, "FAILED TO CREATE WITHOUT SPECIFIC REASON");
                assert(taskCreationResult == pdPASS);
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
                LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
                assert(taskCreationResult == pdPASS);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            default:
                LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
                assert(taskCreationResult == pdPASS);
                return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            }
        }

        LOG_INFO(logger, "Processor has been initialized");
        return Status(Status::Code::GOOD);
    }

    Status Processor::SetBaudRate(const baudrate_e baudRate)
    {
        assert(false);
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status Processor::SetTimeout(const uint16_t timeout)
    {
        assert(false);
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    uint32_t Processor::GetBaudRate() const
    {
        return static_cast<uint32_t>(mBaudRate);
    }

    uint32_t Processor::GetTimeout() const
    {
        return mTimeoutMillis;
    }

    Status Processor::Write(const std::string& command)
    {
        if (xSemaphoreTake(xSemaphore, 1000)  != pdTRUE)
        {
            LOG_WARNING(logger, "THE MODULE IS BUSY. TRY LATER.");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        }

        mSerial.println(command.c_str());
        mSerial.flush();
        xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }

    size_t Processor::GetAvailableBytes()
    {
        return mRxBuffer.GetAvailableBytes();
    }

    int16_t Processor::Read()
    {
        if (xSemaphoreTake(xSemaphore, 10)  != pdTRUE)
        {
            LOG_WARNING(logger, "THE MODULE IS BUSY. TRY LATER.");
            return -1;
        }
        // parseQMTRECV(&rxd);
        // parseQMTSTAT(&rxd);

        int16_t value = mRxBuffer.Read();
        xSemaphoreGive(xSemaphore);
        return value;
    }

    std::string Processor::ReadBetweenPatterns(const std::string& patternBegin, const std::string& patternEnd)
    {
        if (xSemaphoreTake(xSemaphore, 100)  != pdTRUE)
        {
            LOG_WARNING(logger, "THE MODULE IS BUSY. TRY LATER.");
            return "";
        }

        const std::vector<uint8_t> rxd= mRxBuffer.ReadBetweenPatterns(patternBegin, patternEnd);
        xSemaphoreGive(xSemaphore);
        return std::string(rxd.begin(), rxd.end());
    }

    void Processor::stopUrcHandleTask()
    {
        if (mInitFlags.test(init_flags_e::PROCESSOR_TASK_CREATED) == true)
        {
            LOG_INFO(logger, "Stopping URC handling task");
            vTaskDelete(xHandle);
            xHandle = NULL;
        }
    }

    void Processor::implementUrcHandleTask()
    {
    #ifdef DEBUG
        uint32_t checkRemainedStackMillis = millis();
        const uint16_t remainedStackCheckInterval = 60 * 1000;
    #endif

        while (true)
        {
            if (xSemaphoreTake(xSemaphore, 100) != pdTRUE)
            {
                LOG_WARNING(logger, "THE MODULE IS BUSY. TRY LATER");
                continue;
            }

            while (mSerial.available() > 0)
            {
                mRxBuffer.Write(mSerial.read());
            }

            parseRDY();
            parseCFUN();
            parseCPIN();
            parseQIND();
            parseAPPRDY();
            // parseQMTRECV(&rxd);
            // parseQMTSTAT(&rxd);

            xSemaphoreGive(xSemaphore);
            vTaskDelay(mTaskInterval / portTICK_PERIOD_MS);

        #ifdef DEBUG
            if (millis() - checkRemainedStackMillis > remainedStackCheckInterval)
            {
                LOG_DEBUG(logger, "Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
                checkRemainedStackMillis = millis();
            }
        #endif
        }
    }

    void Processor::wrapUrcHandleTask(void* pvParameters)
    {
        static_cast<Processor*>(pvParameters)->implementUrcHandleTask();
    }

    void Processor::parseRDY()
    {
        if (mRxBuffer.HasPattern(urcRDY) == false)
        {
            return;
        }
        else
        {
            if (mRxBuffer.Peek() == 0)
            {
                mRxBuffer.Read();
            }

            mRxBuffer.RemovePattern(urcRDY);
            triggerCallbackRDY();
        }
    }

    void Processor::parseCFUN()
    {
        if (mRxBuffer.HasPattern(urcCFUN) == false)
        {
            return;
        }
        else
        {
            mRxBuffer.RemovePattern(urcCFUN);
            triggerCallbackCFUN();
        }
    }

    void Processor::parseCPIN()
    {
        if (mRxBuffer.HasPattern(urcCPIN) == false)
        {
            return;
        }
        else
        {
            std::vector<uint8_t> rxd = mRxBuffer.ReadBetweenPatterns(urcCPIN, "\r\n");
            const std::string data(rxd.begin(), rxd.end());
            const size_t posStart = data.find(" ") + 1;
            const size_t posFinish = data.find("\r", posStart);
            const std::string param = data.substr(posStart, posFinish-posStart);
            triggerCallbackCPIN(param.c_str());
        }
    }

    void Processor::parseQIND()
    {
        if (mRxBuffer.HasPattern(urcQIND) == false)
        {
            return;
        }
        else
        {
            mRxBuffer.RemovePattern(urcQIND);
            triggerCallbackQIND();
        }
    }

    void Processor::parseAPPRDY()
    {
        if (mRxBuffer.HasPattern(urcAPPRDY) == false)
        {
            return;
        }
        else
        {
            mRxBuffer.RemovePattern(urcAPPRDY);
            triggerCallbackAPPRDY();
        }
    }

/*
    void Processor::parseQMTRECV(std::string* rxd)
    {
        assert(rxd != nullptr);

        if (rxd->find(urcQMTRECV) == std::string::npos)
        {
            return;
        }

        const size_t posStart = rxd->find(urcQMTRECV) + urcQMTRECV.length();
        const size_t posEnd   = rxd->rfind("}\"");

        if (posStart == std::string::npos || posEnd == std::string::npos)
        {
            LOG_ERROR(logger, "INVALID MQTT MESSAGE HAS BEEN FORWARDED");
            rxd->erase(rxd->find(urcQMTRECV), urcQMTRECV.length());
            LOG_DEBUG(logger, "REM: %s", rxd->c_str());

            assert(posStart != std::string::npos);
            assert(posEnd != std::string::npos);
            return;
        }

        const size_t posSocket   = rxd->find(",",   posStart);
        const size_t posMsgID    = rxd->find(",",   posSocket + 1);
        const size_t posTopic    = rxd->find(",",   posMsgID  + 1);
        const size_t posPayload  = posEnd;

        if (posSocket == std::string::npos || posMsgID == std::string::npos ||
            posTopic == std::string::npos  || posPayload == std::string::npos)
        {
            LOG_ERROR(logger, "INVALID URC STRUCTURE: DELIMITER NOT FOUND");
            rxd->erase(posStart, posEnd);
            LOG_DEBUG(logger, "REM: %s", rxd->c_str());

            assert(posSocket != std::string::npos);
            assert(posMsgID != std::string::npos);
            assert(posTopic != std::string::npos);
            assert(posPayload != std::string::npos);
            return;
        }

        size_t length = posSocket - posStart;
        std::string token = rxd->substr(posStart, length);
        LOG_VERBOSE(logger, "%s", token.c_str());
        const uint8_t socket = std::stoi(token);
        LOG_VERBOSE(logger, "socket: %u", socket);

        length = posMsgID - posSocket;
        token = rxd->substr(posSocket + 1, length);
        LOG_VERBOSE(logger, "%s", token.c_str());
        const uint16_t msgID = std::stoi(token);
        LOG_VERBOSE(logger, "msgID: %u", msgID);

        length = posTopic - posMsgID - 3;
        const std::string strTopic = rxd->substr(posMsgID + 2, length);
        LOG_VERBOSE(logger, "%s", strTopic.c_str());
        const MqttTopicEnum topic = StringToTopic(strTopic.c_str());

        length = posEnd - posTopic - 1;
        LOG_VERBOSE(logger, "End: %u   Topic: %u", posEnd, posTopic);
        const std::string payload = rxd->substr(posTopic + 2, length);
        LOG_VERBOSE(logger, "%s", payload.c_str());

        // Send the message to the FreeRTOS queue
        uint8_t numItemsInQueue = uxQueueMessagesWaiting(QUEUE_MQTT_MESSAGE);
        LOG_VERBOSE(logger, "Number of items in queue: %u", numItemsInQueue);

        MqttMessage* message = new(std::nothrow) MqttMessage(
            socket, msgID, topic, payload.c_str()
        );
        if (message == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        xQueueSend(QUEUE_MQTT_MESSAGE, &message, portMAX_DELAY);
        LOG_VERBOSE(logger, "Sent the message to the queue");

        numItemsInQueue = uxQueueMessagesWaiting(QUEUE_MQTT_MESSAGE);
        LOG_VERBOSE(logger, "Number of items in queue: %u", numItemsInQueue);

        rxd->erase(posStart, posEnd);
        removeLeadingCRLF(rxd);
        
        LOG_VERBOSE(logger, "Remained: %s", rxd->c_str());
        void triggerCallbackQMTRECV();
    }
*/

/*
    void Processor::parseQMTSTAT(std::string* rxd)
    {
        assert(rxd != nullptr);

        if (rxd->find(urcQMTSTAT) == std::string::npos)
        {
            return;
        }

        const size_t posStart  = rxd->find(urcQMTSTAT) + urcQMTSTAT.length();
        const size_t delimiter = rxd->rfind(",", posStart + 1);
        LOG_DEBUG(logger, "QMTSTAT URC: %s", rxd->substr(posStart, delimiter - posStart + 1).c_str());

        if (delimiter == std::string::npos)
        {
            LOG_ERROR(logger, "INVALID MQTT LINK LAYER URC RECEIVED");
            rxd->erase(rxd->find(urcQMTSTAT), urcQMTSTAT.length());
            LOG_DEBUG(logger, "REM: %s", rxd->c_str());

            assert(delimiter != std::string::npos);
            return;
        }

        const uint8_t socketID  = std::stoi(rxd->substr(delimiter - 1, 1));
        const uint8_t errorCode = std::stoi(rxd->substr(delimiter + 1, 1));

        switch (errorCode)
        {
        case 1:
            LOG_ERROR(logger, "CONNECTION IS CLOSED OR RESET BY THE PEER");
            break;
        case 2:
            LOG_ERROR(logger, "SENDING PINGREQ PACKET TIMED OUT OR FAILED");
            break;
        case 3:
            LOG_ERROR(logger, "SENDING CONNECT PACKET TIMED OUT OR FAILED");
            break;
        case 4:
            LOG_ERROR(logger, "RECEIVING CONNACK PACKET TIMED OUT OR FAILED");
            break;
        case 5:
            LOG_ERROR(logger, "DISCONNECT PACKET IS SENT BY THE APP");
            break;
        case 6:
            LOG_ERROR(logger, "DISCONNECT FOR PACKETS FAIL ALL THE TIME");
            break;
        case 7:
            LOG_ERROR(logger, "THE LINK IS NOT ALIVE OR BROKER UNAVAILABLE");
            break;
        default:
            LOG_ERROR(logger, "PROCESSOR ERROR: UNDEFINED ERROR CODE: %d", errorCode);
            return;
        }
        // triggerCallbackQMTSTAT(socketID, errorCode);
    }
*/

    void Processor::RegisterCallbackRDY(const std::function<void()>& cb)
    {
        mCallbackRDY = cb;
    }

    void Processor::RegisterCallbackCFUN(const std::function<void()>& cb)
    {
        mCallbackCFUN = cb;
    }

    void Processor::RegisterCallbackCPIN(const std::function<void(const std::string&)>& cb)
    {
        mCallbackCPIN = cb;
    }

    void Processor::RegisterCallbackQIND(const std::function<void()>& cb)
    {
        mCallbackQIND = cb;
    }

    void Processor::RegisterCallbackAPPRDY(const std::function<void()>& cb)
    {
        mCallbackAPPRDY = cb;
    }

    void Processor::triggerCallbackRDY()
    {
        if (mCallbackRDY != nullptr)
        {
            mCallbackRDY();
        }
        else
        {
            LOG_ERROR(logger, "CALLBACK IS NOT REGISTERED");
            assert(mCallbackRDY);
        }
    }

    void Processor::triggerCallbackCFUN()
    {
        if (mCallbackCFUN != nullptr)
        {
            mCallbackCFUN();
        }
        else
        {
            LOG_ERROR(logger, "CALLBACK IS NOT REGISTERED");
            assert(mCallbackCFUN);
        }
    }

    void Processor::triggerCallbackCPIN(const std::string& state)
    {
        if (mCallbackCPIN != nullptr)
        {
            mCallbackCPIN(state);
        }
        else
        {
            LOG_ERROR(logger, "CALLBACK IS NOT REGISTERED");
            assert(mCallbackCPIN);
        }
    }

    void Processor::triggerCallbackQIND()
    {
        if (mCallbackQIND != nullptr)
        {
            mCallbackQIND();
        }
        else
        {
            LOG_ERROR(logger, "CALLBACK IS NOT REGISTERED");
            assert(mCallbackQIND);
        }
    }

    void Processor::triggerCallbackAPPRDY()
    {
        if (mCallbackAPPRDY != nullptr)
        {
            mCallbackAPPRDY();
        }
        else
        {
            assert(mCallbackAPPRDY);
        }
    }

    // void Processor::triggerCallbackQMTRECV()
    // {
    //     if (mCallbackQMTRECV)
    //     {
    //         mCallbackQMTRECV();
    //     }
    //     else
    //     {
    //         assert(mCallbackQMTRECV);
    //     }
    // }

    // void Processor::triggerCallbackQMTSTAT(const uint8_t socketID, 
    //                                        const uint8_t errorCode)
    // {
    //     if (mCallbackQMTSTAT)
    //     {
    //         mCallbackQMTSTAT(socketID, errorCode);
    //     }
    //     else
    //     {
    //         assert(mCallbackQMTSTAT);
    //     }
    // }
}