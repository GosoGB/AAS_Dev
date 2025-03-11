/**
 * @file SPEAR.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief SPEAR 프로토콜 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#if defined(MODLINK_T2) || defined(MODLINK_B)

#pragma once

#include <stdint-gcc.h>
#include <queue>
#include <ArduinoJson.h>

#include "freertos/semphr.h"
#include "Common/Status.h"
#include "Include/TypeDefinitions.h"
#include "JARVIS/Include/Base.h"
#include "Protocol/Modbus/Include/AddressTable.h"



namespace muffin {

    extern SemaphoreHandle_t xSemaphoreSPEAR;

    class SPEAR
    {
    public:
        SPEAR() {}
        virtual ~SPEAR() {}
    public:
        Status Init();
        Status Reset();
    
    public:
        Status Send(const char payload[]);
        Status Receive(const uint16_t timeoutMillis, const size_t size, char payload[]);
    private:
        uint8_t calculateChecksum(const char payload[]);
        Status buildMessage(const char payload[], char output[]);

    /**
     * @brief StatusServiceSets
     */
    public:
        Status VersionEnquiryService();
        Status MemoryEnquiryService();
        Status StatusEnquiryService();
        Status SignOnService();

    /**
     * @brief JarvisServiceSets
     */
    public:
        Status SetJarvisLinkConfig(jvs::config::Base* cin, const jvs::cfg_key_e type);
        Status SetJarvisProtocolConfig(const std::set<jvs::prt_e> link);
    private:
        Status setJarvisRs485Config(jvs::config::Base* cin);
        Status validateSetService();
    public:
        Status resendSetService();
        Status resendSetConfig(const std::string& path);
    private:
        Status receiveSignOn();
    private:
        void writeJson(const char payload[], const std::string& path);
        Status readJson(const std::string& path, const uint8_t size, char payload[]);

    /**
     * @brief DaqServiceSets
     */
    public:
        Status PollService(spear_daq_msg_t* daq);

    /**
     * @brief RcServiceSets
     */
    public:
        Status ExecuteService(spear_remote_control_msg_t msg);

    private:
        static const uint8_t STX = 0x02;
        static const uint8_t ETX = 0x03;
        static const uint8_t HEAD_SIZE = 0x03;
        static const uint8_t TAIL_SIZE = 0x02;
        static const uint8_t RESET_PIN = 13;
    private:
        // SemaphoreHandle_t xSemaphore;
    };


    extern SPEAR spear;
}

#endif