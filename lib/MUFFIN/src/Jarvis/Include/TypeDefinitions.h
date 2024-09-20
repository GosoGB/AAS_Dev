/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-10-05
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <sys/_stdint.h>



namespace muffin { namespace jarvis {

    typedef enum class JarvisProtocolVersionEnum
        : uint8_t
    {
        VERSEOIN_1 = 1,
        VERSEOIN_2 = 2,
        VERSEOIN_3 = 3,
    } prtcl_ver_e;

    typedef enum class JarvisConfigClassKeyEnum
        : uint8_t
    {
        RS232               =  0,
        RS485               =  1,
        WIFI4               =  2,
        ETHERNET            =  3,
        LTE_CatM1           =  4,
        MODBUS_RTU          =  5,
        MODBUS_TCP          =  6,
        OPERATION           =  7,
        NODE                =  8,
        ALARM               =  9,
        OPERATION_TIME      = 10,
        PRODUCTION_INFO     = 11
    } cfg_key_e;

    typedef enum ModlinkSerialPortIndexEnum
        : uint8_t
    {
        PORT_2 = 2,
    #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        PORT_3 = 3
    #endif
    } prt_e;

    typedef enum ModlinkSerialPortBaudRateEnum
        : uint32_t
    {
        BDR_9600    =   9600,
        BDR_19200   =  19200,
        BDR_38400   =  38400,
        BDR_115200  = 115200
    } bdr_e;

    typedef enum ModlinkSerialPortDataBitEnum
        : uint8_t
    {
        DBIT_5  = 5,
        DBIT_6  = 6,
        DBIT_7  = 7,
        DBIT_8  = 8
    } dbit_e;

    typedef enum ModlinkSerialPortParityBitEnum
        : uint8_t
    {
        NONE  = 0,
        ODD   = 1,
        EVEN  = 2
    } pbit_e;

    typedef enum ModlinkSerialPortStopBitEnum
        : uint8_t
    {
        SBIT_1  = 1,
        SBIT_2  = 2
    } sbit_e;
    
    typedef enum ModlinkNetworkInterfaceEnum
        : uint8_t
    {
        ETHERNET  = 0,
        WIFI4     = 1,
        LTE_CatM1 = 2
    } nic_e;
}}