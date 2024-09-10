/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 모듈에서 공통적으로 사용하는 데이터 타입들을 정의합니다.
 * 
 * @date 2024-09-05
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