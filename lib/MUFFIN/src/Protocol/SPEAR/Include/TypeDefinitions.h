/**
 * @file SpearFormat.h
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

#include <vector>
#include <stdint-gcc.h>
#include "JARVIS/Include/TypeDefinitions.h"


namespace muffin {
    
    typedef enum class SpearConfigTypeEnum 
        : uint8_t
    {
        LINK1      = 0x01,
        LINK2      = 0x02,
        PROTOCOL   = 0x03,
    } cfg_type_e;

    typedef enum class SpearModbusStatusEnum 
        : uint16_t
    {
        SUCCESS                                    = 0x0,        
        I_O_ERROR                                  = 0x5,
        INVALID_ARGUMENT                           = 0x16,
        PROTOCOL_NOT_AVAILABLE                     = 0x6D,
        CONNECTION_REFUSED                         = 0x6F,
        CONNECTION_TIMEOUT                         = 0x74,
        NOT_SUPPORTED                              = 0x86,
        ILLEGAL_FUNCTION                           = 0x2BE3,
        ILLEGAL_DATA_ADDRESS                       = 0x2BE4,
        ILLEGAL_DATA_VALUE                         = 0x2BE5,
        SLAVE_OR_SERVER_FAILURE                    = 0x2BE6,
        ACKNOWLEDGE_EXCEPTION                      = 0x2BE7,
        SLAVE_OR_SERVER_BUSY                       = 0x2BE8,
        NEGATIVE_ACKNOWLEDGE                       = 0x2BE9,
        MEMORY_PARITY_EXCEPTION                    = 0x2BEA,
        UNDEFINED_EXCEPTION                        = 0x2BEB,
        GATEWAY_PATH_NOT_AVAILABLE                 = 0x2BEC,
        TARGET_DEVICE_FAILED_TO_RESPOND            = 0x2BED,
        INVALID_CRC_CHECK_VALUE                    = 0x2BEE,
        INVALID_DATA_VALUES                        = 0x2BEF,
        INVALID_EXCEPTION_CODE                     = 0x2BF0,
        TOO_MANY_DATA                              = 0x2BF1,
        RESPONSE_ISN_T_FROM_THE_REQUESTED_SLAVE    = 0x2BF2
    } mb_status_e;

    typedef enum class SpearConfigKeyEnum 
        : uint8_t
    {
        RS232      = 0x11,
        RS485      = 0x12,
        RTD        = 0x13,
        NTC        = 0x14,
        MODBUS     = 0x61,
        MELSEC     = 0x62,
      
    } cfg_key_e;

    typedef enum class SpearCommandEnum 
        : uint8_t
    {
        SIGN_ON_REQUEST    = 0x01,
        VERSION_ENQUIRY    = 0x02,
        MEMORY_ENQUIRY     = 0x03,
        STATUS_ENQUIRY     = 0x04,
        JARVIS_SETUP       = 0x11,
        // JARVIS_RESET       = 0x12,
        DAQ_POLL           = 0x21,
        // DAQ_STOP           = 0x22,
        // DAQ                = 0x23,
        REMOTE_CONTROL     = 0X31
    } spear_cmd_e;

    typedef struct RequestMessageFormatHeadType
    {
        uint8_t Code;
        uint8_t Index;
    } req_head_t;

    typedef struct StartMessageFormatHeadType
    {
        uint8_t Code;
        uint8_t Index;
        uint16_t pollingInterval;
    } req_start_head_t;

    typedef struct ResponseMessageFormatHeadType
    {
        uint8_t Code;
        uint8_t Index;
        uint32_t Status;
    } resp_head_t;

    typedef struct ResponseVersionEnquiryServiceType
    {
        resp_head_t Head;
        char SemanticVersion[16];
        uint32_t VersionCode;
    } resp_vsn_t;

    typedef struct ResponseMemoryEnquiryServiceType
    {
        resp_head_t Head;
        uint32_t Remained;
        uint32_t UsedStack;
        uint32_t UsedHeap;
    } resp_mem_t;

    typedef struct ResponseStatusEnquiryServiceType
    {
        resp_head_t Head;
        uint32_t StatusCode;
    } resp_status_t;

    typedef struct SpearMessageType
    {
        uint32_t Length;
        char* Frame;
    } spear_msg_t;

    typedef struct SpearDaqMessageType
    {
        jvs::prt_e Link;
        uint8_t SlaveID;
        jvs::mb_area_e Area;
        uint16_t Address;
        uint16_t Quantity;
        std::vector<int32_t> PolledValuesVector;
    } spear_daq_msg_t;

    typedef struct SpearRemoteControlMessageType
    {
        resp_head_t Head;
        jvs::prt_e Link;
        uint8_t SlaveID;
        jvs::mb_area_e Area;
        uint16_t Address;
        uint16_t Value;
    } spear_remote_control_msg_t;
    
}