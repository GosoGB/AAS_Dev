/**
 * @file Convert.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다양한 데이터 타입 간의 변환을 지원하는 유틸리티 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin {

namespace jvs { namespace config {
    class Base;
    /*Information*/
    class Alarm;
    class Node;
    class OperationTime;
    class Production;
    /*Interfaces*/
    class Rs232;
    class Rs485;
    /*Network*/
    class CatM1;
    class Ethernet;
    class WiFi4;
    /*Operation*/
    class Operation;
    /*Protocol*/
    class ModbusRTU;
    class ModbusTCP;
}}


    class ConvertClass
    {
    public:
        static int8_t ToInt8(const std::string& input);
        static int16_t ToInt16(const std::string& input);
        static int32_t ToInt32(const std::string& input);
        static int64_t ToInt64(const std::string& input);
    public:
        static uint8_t ToUInt8(const std::string& input);
        static uint8_t ToUInt8(const jvs::cfg_key_e input);
        static uint8_t ToUInt8(const jvs::prtcl_ver_e input);
        static uint16_t ToUInt16(const std::string& input);
        static uint16_t ToUInt16(const jvs::rsc_e input);
        static uint32_t ToUInt32(const std::string& input);
        static uint32_t ToUInt32(const time_t input);
        static uint32_t ToUInt32(const jvs::bdr_e input);
        static uint64_t ToUInt64(const std::string& input);
    public:
        static float ToFloat(const std::string& input);
        static double ToDouble(const std::string& input);
    public:
        template <typename T>
        static std::string ToString(const T& input);
        static std::string ToString(const jvs::cfg_key_e input);
        static std::string ToString(const jvs::prtcl_ver_e input);
    public:
        static std::pair<Status, jvs::cfg_key_e> ToJarvisKey(const jvs::prtcl_ver_e version, const std::string& input);
        static std::pair<Status, jvs::prtcl_ver_e> ToJarvisVersion(const std::string& input);
        static jvs::config::Alarm* ToAlarmCIN(jvs::config::Base* config);
        static jvs::config::Node* ToNodeCIN(jvs::config::Base* config);
        static jvs::config::OperationTime* ToOperationTimeCIN(jvs::config::Base* config);
        static jvs::config::Production* ToProductionCIN(jvs::config::Base* config);
        static jvs::config::Rs232* ToRS232CIN(jvs::config::Base* config);
        static jvs::config::Rs485* ToRS485CIN(jvs::config::Base* config);
        static jvs::config::CatM1* ToCatM1CIN(jvs::config::Base* config);
        static jvs::config::Ethernet* ToEthernetCIN(jvs::config::Base* config);
        static jvs::config::WiFi4* ToWiFi4CIN(jvs::config::Base* config);
        static jvs::config::Operation* ToOperationCIN(jvs::config::Base* config);
        static jvs::config::ModbusRTU* ToModbusRTUCIN(jvs::config::Base* config);
        static jvs::config::ModbusTCP* ToModbusTCPCIN(jvs::config::Base* config);
    };

    template <typename T>
    std::string ConvertClass::ToString(const T& input)
    {
        static_assert(std::is_integral<T>::value, "ONLY INTEGRAL TYPES ARE SUPPORTED");
        return std::to_string(input);
    }


    extern ConvertClass Convert;
}