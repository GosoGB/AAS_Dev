/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 모듈에서 공통적으로 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin { namespace network {

    typedef struct WirelessSignalQualityType
    {
        int16_t  RSSI;
        int16_t  RSRP;
        int16_t  SINR;
        int16_t  RSRQ;
    } wl_quality_t;

    typedef struct PingTestResultType
    {
        uint8_t NumberPassed;
        uint8_t NumberFailed;
        uint16_t ResponseTime[4];
        uint16_t MaxResponseTime;
        uint16_t MinResponseTime;
        float AverageResponseTime;
    } ping_result_t;
}}


namespace muffin { namespace network { namespace lte {

    typedef enum class PdpContextEnum
        : uint8_t
    {
        PDP_01  =  1,
        PDP_02  =  2,
        PDP_03  =  3,
        PDP_04  =  4,
        PDP_05  =  5,
        PDP_06  =  6,
        PDP_07  =  7,
        PDP_08  =  8,
        PDP_09  =  9,
        PDP_10  = 10,
        PDP_11  = 11,
        PDP_12  = 12,
        PDP_13  = 13,
        PDP_14  = 14,
        PDP_15  = 15,
        PDP_16  = 16
    } pdp_ctx_e;

    typedef enum class SslContextEnum
        : uint8_t
    {
        SSL_0  =  0,
        SSL_1  =  1,
        SSL_2  =  2,
        SSL_3  =  3,
        SSL_4  =  4,
        SSL_5  =  5
    } ssl_ctx_e;

    typedef enum class SslVersionEnum
        : uint8_t
    {
        SSL_3_0 = 0,
        TLS_1_0 = 1,
        TLS_1_1 = 2,
        TLS_1_2 = 3,
        ALL_SSL = 4
    } ssl_vsn_e;

    typedef enum class SslCipherSuiteEnum
        : uint16_t
    {
        TLS_RSA_WITH_AES_256_CBC_SHA          = 0x0035,
        TLS_RSA_WITH_AES_128_CBC_SHA          = 0x002F,
        TLS_RSA_WITH_RC4_128_SHA              = 0x0005,
        TLS_RSA_WITH_RC4_128_MD5              = 0x0004,
        TLS_RSA_WITH_3DES_EDE_CBC_SHA         = 0x000A,
        TLS_RSA_WITH_AES_256_CBC_SHA256       = 0x003D,
        TLS_ECDHE_RSA_WITH_RC4_128_SHA        = 0xC011,
        TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA   = 0xC012,
        TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA    = 0xC013,
        TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA    = 0xC014,
        TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 = 0xC027,
        TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 = 0xC028,
        TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 = 0xC02F,
        ALL_MQTT_CIPHER_VERSION               = 0xFFFF
    } ssl_cipher_e;
}}}