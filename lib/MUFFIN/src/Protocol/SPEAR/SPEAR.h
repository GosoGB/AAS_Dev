/**
 * @file SPEAR.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2024-12-11
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <stdint-gcc.h>
#include <queue>
#include <ArduinoJson.h>

#include "freertos/semphr.h"
#include "Common/Status.h"
#include "Include/TypeDefinitions.h"
#include "Jarvis/Include/Base.h"
#include "Protocol/Modbus/Include/AddressTable.h"


namespace muffin {

    extern SemaphoreHandle_t xSemaphoreSPEAR;
    class SPEAR
    {
    public:
        SPEAR() {}
        virtual ~SPEAR() {}
    public:
        Status reset();
        Status Init();
    public:
        Status VersionEnquiryService();
        Status MemoryEnquiryService();
        Status StatusEnquiryService();
        Status ExecuteService(spear_remote_control_msg_t msg);
        Status PollService(spear_daq_msg_t* daq);
    public:
        Status SetJarvisLinkConfig(jarvis::config::Base* cin, const jarvis::cfg_key_e type);
        Status SetJarvisProtocolConfig(const std::set<jarvis::prt_e> link);
    private:
        Status setJarvisRs485Config(jarvis::config::Base* cin);
    public:
        Status resendSetService();
    private:
        Status receiveSignOn();
    private:
        Status resendSetConfig(const std::string& path);
    private:
        void writeJson(const char payload[],const std::string& path);
        Status readJson(const std::string& path, std::string* payload);

    private:
        Status validateSetService();
    public:
        uint8_t Count() const;
        Status Peek(spear_msg_t* message);
        Status Retrieve(spear_msg_t* message);
        void Send(char payload[]);
        void Receive();
    private:
        uint8_t calculateChecksum(char payload[]);
        uint8_t calculateChecksum(const spear_msg_t& message);
        void buildMessage(char payload[]);
        void send();
    private:
        uint8_t mSequenceID = 0x00;
    private:
        static const uint8_t STX = 0x02;
        static const uint8_t ETX = 0x03;
        static const uint8_t HEAD_SIZE = 0x03;
        static const uint8_t TAIL_SIZE   = 0x02;
    private:
        std::queue<spear_msg_t*> mQueueRxD;
        std::queue<spear_msg_t*> mQueueTxD;
        const uint8_t MAX_TRIAL_COUNT = 5;
        const uint8_t RESET_PIN = 13;
    };
}