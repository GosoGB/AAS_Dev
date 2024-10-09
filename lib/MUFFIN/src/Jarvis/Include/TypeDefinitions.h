/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS에서 사용하는 데이터 타입들을 정의합니다.
 * 
<<<<<<< HEAD
 * @date 2024-10-05
=======
 * @date 2024-10-09
>>>>>>> feature/jarvis/config
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <sys/_stdint.h>



namespace muffin { namespace jarvis {

<<<<<<< HEAD
=======
    typedef enum class NodeAddressTypeEnum
        : uint8_t
    {
        NUMERIC     = 1,
        STRING      = 2,
        BYTE_STRING = 3,
        GUID        = 4
    } adtp_e;

    typedef union NodeAddressUnion
    {
        uint32_t Numeric;
    } addr_u;

    typedef enum class ModbusAreaEnum
        : uint8_t
    {
        COILS            = 1,
        DISCRETE_INPUT   = 2,
        INPUT_REGISTER   = 3,
        HOLDING_REGISTER = 4
    } mb_area_e;

    typedef enum class NumericScaleByPowerOfTenEnum
        : int8_t
    {
        NEGATIVE_3   = -3,
        NEGATIVE_2   = -2,
        NEGATIVE_1   = -1,
        POSITIVE_1   =  1,
        POSITIVE_2   =  2,
        POSITIVE_3   =  3
    } scl_e;

    typedef enum class DataTypeEnum
        : uint8_t
    {
        BOOLEAN   =  0,
        INT8      =  1,
        UINT8     =  2,
        INT16     =  3,
        UINT16    =  4,
        INT32     =  5,
        UINT32    =  6,
        INT64     =  7,
        UINT64    =  8,
        FLOAT32   =  9,
        FLOAT64   = 10,
        STRING    = 11
    } dt_e;

>>>>>>> feature/jarvis/config
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
<<<<<<< HEAD
=======

    typedef enum AlarmEventTypeEnum
        : uint8_t
    {
        ONLY_LCL      = 1,
        ONLY_UCL      = 2,
        LCL_AND_UCL   = 3,
        ON_CONDITION  = 4
    } alarm_type_e;

    typedef enum OperationTimeTypeEnum
        : uint8_t
    {
        FROM_MACHINE  = 1,
        FROM_MODLINK  = 2
    } op_time_type_e;

    typedef enum ComparisonOperatorEnum
        : uint8_t
    {
        LESS_THAN     = 0,
        LESS_EQUAL    = 1,
        EQUAL         = 2,
        GREATER_EQUAL = 3,
        GREATER_THAN  = 4
    } cmp_op_e;

    typedef enum class LteCatM1ModelEnum
    {
        LM5,
        LCM300
    } md_e;

    typedef enum class LteCatM1CountryEnum
    {
        KOREA,
        USA
        // JAPAN,
        // VIETNAM,
        // TAIWAN
    } ctry_e;

    typedef enum class WiFi4AuthModeEnum
    {
        OPEN               = 0,
        WEP                = 1,
        WPA_PSK            = 2,
        WPA2_PSK           = 3,
        WPA_WPA2_PSK       = 4,
        WPA2_ENTERPRISE    = 5
    } wifi_auth_e;

    typedef enum class WiFi4ExtensibleAuthenticationProtocolEnum
    {
        TLS   = 0,
        PEAP  = 1,
        TTLS  = 2
    } wifi_eap_auth_e;

    typedef enum class ServerNetworkInterfaceCardEnum
    {
        WiFi4     = 0,
        Ethernet  = 1,
        LTE_CatM1 = 2
    } snic_e;

    typedef enum class DataUnitEnum
        : uint8_t
    {
        BYTE   =  8,
        WORD   = 16,
        DWORD  = 32,
        QWORD  = 64
    } data_unit_e;

    typedef enum class ByteOrderEnum
        : uint8_t
    {
        LOW   = 0,
        HIGH  = 1
    } byte_order_e;

    typedef struct DataUnitOrderType
    {
        data_unit_e DataUnit;
        byte_order_e ByteOrder;
        uint8_t Index;
    } ord_t;

    typedef enum class FormatSpecifierEnum
    {
        INTEGER_32,
        UNSIGNED_INTEGER_32,
        FLOATING_POINT_32,
        STRING,
        CHARACTER,
        HEX_LOWERCASE,
        HEX_UPPERCASE
    } fmt_spec_e;
>>>>>>> feature/jarvis/config
}}