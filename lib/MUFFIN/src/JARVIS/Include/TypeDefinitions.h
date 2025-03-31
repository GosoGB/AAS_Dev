/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS에서 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-10-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <sys/_stdint.h>



namespace muffin { namespace jvs {

    typedef enum class ResponseStatusCodeEnum
        : uint16_t
    {
        GOOD                                        = 0x0000,
        GOOD_NO_DATA                                = 0x0001,
        UNCERTAIN                                   = 0x4000,
        UNCERTAIN_CONFIG_INSTANCE                   = 0x4460,
        UNCERTAIN_CONFIG_INSTANCES                  = 0x44A0,
        UNCERTAIN_VERSION_CONFIG_INSTANCE           = 0x4560,
        UNCERTAIN_VERSION_CONFIG_INSTANCES          = 0x45A0,
        BAD                                         = 0x8000,
        BAD_UNEXPECTED_ERROR                        = 0x8218,
        BAD_COMMUNICATION                           = 0x8240,
        BAD_COMMUNICATION_TIMEOUT                   = 0x8250,
        BAD_COMMUNICATION_CAPACITY_EXCEEDED         = 0x8260,
        BAD_DECODING_ERROR                          = 0x8280,
        BAD_DECODING_CAPACITY_EXCEEDED              = 0x82A0,
        BAD_INVALID_FORMAT_CONFIG_INSTANCE          = 0x8450,
        BAD_INVALID_FORMAT_CONFIG_INSTANCES         = 0x8490,
        BAD_INVALID_PRECONDITION_OR_RELATION        = 0x84A0,
        BAD_INVALID_VERSION                         = 0x8500,
        BAD_INVALID_VERSION_CONFIG_INSTANCE         = 0x8550,
        BAD_INVALID_VERSION_CONFIG_INSTANCES        = 0x8590,
        BAD_INVALID_VERSION_PRECONDITION_RELATION   = 0x85A0,
        BAD_INTERNAL_ERROR                          = 0x8810,
        BAD_OUT_OF_MEMORY                           = 0x8820,
        BAD_UNSUPPORTED_CONFIGURATION               = 0x8840,
        BAD_TEMPORARY_UNAVAILABLE                   = 0x8880,
        BAD_HARDWARE_FAILURE                        = 0x8900
    } rsc_e;

    typedef enum class NodeAddressTypeEnum
        : uint8_t
    {
        NUMERIC     = 0,
        STRING      = 1,
        BYTE_STRING = 2,
        GUID        = 3
    } adtp_e;

    typedef union NodeAddressUnion
    {
        uint32_t Numeric;
    } addr_u;

    typedef enum class NodeAreaEnum
        : uint16_t
    {
        COILS            = 1,
        DISCRETE_INPUT   = 2,
        INPUT_REGISTER   = 3,
        HOLDING_REGISTER = 4,
        SM	             = 5,
        SD	             = 6,	
        X	             = 7,	
        Y	             = 8,	
        M	             = 9,	
        L	             = 10,	
        F	             = 11, 
        V	             = 12,
        B	             = 13,
        D	             = 14,
        W	             = 15,
        TS	             = 16,
        TC	             = 17,
        TN	             = 18,
        LTS	             = 19,
        LTC	             = 20,
        LTN	             = 21,
        STS	             = 22,
        STC	             = 23,
        STN	             = 24,
        LSTS	         = 25,
        LSTC	         = 26,
        LSTN	         = 27,
        CS	             = 28,
        CC	             = 29,
        CN	             = 30,
        LCS	             = 31,
        LCC	             = 32,
        LCN	             = 33,
        SB	             = 34,
        SW	             = 35,
        S	             = 36,
        DX	             = 37,
        DY	             = 38,
        Z	             = 39,
        LZ	             = 40
    } node_area_e;

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

    typedef enum class JarvisProtocolVersionEnum
        : uint8_t
    {
        VERSEOIN_1 = 1,
        VERSEOIN_2 = 2,
        VERSEOIN_3 = 3,
        VERSEOIN_4 = 4
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
        PRODUCTION_INFO     = 11,
        MELSEC              = 12
    } cfg_key_e;

    typedef enum class ModlinkSerialPortIndexEnum
        : uint8_t
    {
        PORT_2 = 2,
    #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        PORT_3 = 3
    #endif
    } prt_e;

    typedef enum class ModlinkSerialPortBaudRateEnum
        : uint32_t
    {
        BDR_9600    =   9600,
        BDR_19200   =  19200,
        BDR_38400   =  38400,
        BDR_115200  = 115200
    } bdr_e;

    typedef enum class ModlinkSerialPortDataBitEnum
        : uint8_t
    {
        DBIT_5  = 5,
        DBIT_6  = 6,
        DBIT_7  = 7,
        DBIT_8  = 8
    } dbit_e;

    typedef enum class ModlinkSerialPortParityBitEnum
        : uint8_t
    {
        NONE  = 0,
        ODD   = 1,
        EVEN  = 2
    } pbit_e;

    typedef enum class ModlinkSerialPortStopBitEnum
        : uint8_t
    {
        SBIT_1  = 1,
        SBIT_2  = 2
    } sbit_e;
    
    typedef enum class ModlinkNetworkInterfaceEnum
        : uint8_t
    {
        ETHERNET  = 0,
        WIFI4     = 1,
        LTE_CatM1 = 2
    } nic_e;

    typedef enum class MitsubishiPlcSeriesEnum
        : uint8_t
    {
        QL_SERIES  = 0,
        IQR_SERIES = 1
    } ps_e;

    typedef enum class MelsecDataFormatEnum
        : uint8_t
    {
        BINARY  = 0,
        ASCII   = 1
    } df_e;

    typedef enum class AlarmEventTypeEnum
        : uint8_t
    {
        ONLY_LCL      = 1,
        ONLY_UCL      = 2,
        LCL_AND_UCL   = 3,
        CONDITION     = 4
    } alarm_type_e;

    typedef enum class OperationTimeTypeEnum
        : uint8_t
    {
        FROM_MACHINE  = 1,
        FROM_MODLINK  = 2
    } op_time_type_e;

    typedef enum class ComparisonOperatorEnum
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

    typedef enum class ServerNetworkInterfaceCardEnum : uint8_t
    {
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        Ethernet,
    #elif defined(MODLINK_B)
        WiFi4,
    #endif
        LTE_CatM1
    } snic_e;

    typedef enum class DataUnitEnum : uint8_t
    {
        BYTE   =  8,
        WORD   = 16,
        DWORD  = 32,
        QWORD  = 64
    } data_unit_e;

    typedef enum class ByteOrderEnum
        : uint8_t
    {
        LOWER   = 0,
        HIGHER  = 1
    } byte_order_e;

    typedef struct DataUnitOrderType
    {
        data_unit_e DataUnit;
        byte_order_e ByteOrder;
        uint8_t Index;
    } ord_t;

    typedef enum class FormatSpecifierEnum
        : uint8_t
    {
        INTEGER_32,
        INTEGER_64,
        UNSIGNED_INTEGER_32,
        UNSIGNED_INTEGER_64,
        FLOATING_POINT_64,
        CHARACTER,
        STRING,
        HEX_LOWERCASE,
        HEX_UPPERCASE
    } fmt_spec_e;

    typedef enum class OperationStatus
        : uint8_t
    {
        PROCESSING,
        IDLE,
        ERROR,
        TURN_OFF
    } op_status_e;

    
}}