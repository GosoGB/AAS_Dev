/**
 * @file JSON.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 선언합니다.
 * 
 * @date 2025-02-10
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo 인코딩 함수를 추가로 구현해야 합니다.
 */




#pragma once

#include <ArduinoJson.h>
#include <FS.h>
#include <vector>

#include "Common/Status.h"
#include "Protocol/MQTT/Include/TypeDefinitions.h"
#include "Protocol/SPEAR/Include/TypeDefinitions.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin {

    typedef struct FotaStatusStruct
    {
        mqtt::topic_e Topic;
        uint32_t VersionCodeMcu1;
        uint32_t VersionCodeMcu2;
        std::string VersionMcu1;
        std::string VersionMcu2;
    } fota_status_t;

    typedef struct JarvisStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        uint32_t ResponseCode;
        std::string Description;
        std::vector<std::string> Config;
    } jarvis_struct_t;
    typedef struct JarvisRs485Struct
    {
        jvs::prt_e PortIndex;
        jvs::bdr_e BaudRate;
        jvs::dbit_e DataBit;
        jvs::pbit_e ParityBit;
        jvs::sbit_e StopBit;
    } jarvis_rs485_struct_t;

    typedef struct JarvisEthernetStruct
    {
        bool IsEthernetSet = false;
        bool EnableDHCP;
        IPAddress StaticIPv4;
        IPAddress Subnetmask;
        IPAddress Gateway;
        IPAddress DNS1;
        IPAddress DNS2;
    } jarvis_ethernet_struct_t;

    typedef struct JarvisCatm1Struct
    {
        bool IsCatM1Set = false;
        jvs::md_e Model;
        jvs::ctry_e Country;
    } jarvis_catm1_struct_t;

    typedef struct JarvisInterfaceStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        jvs::snic_e SNIC;
        std::vector<jarvis_rs485_struct_t> RS485;
        jarvis_ethernet_struct_t Ethernet;
        jarvis_catm1_struct_t CatM1;
    } jarvis_interface_struct_t;

    typedef struct RemoteControllStruct
    {
        mqtt::topic_e Topic;
        std::string ID;
        uint64_t SourceTimestamp;
        std::string ResponseCode;
        JsonArray RequestData;
    } remote_controll_struct_t;

    typedef struct DaqStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        std::string Uid;
        std::string Value;
    } daq_struct_t;

    typedef struct AlarmStruct
    {
        mqtt::topic_e Topic;
        std::string AlarmType;
        uint64_t AlarmStartTime;
        int64_t AlarmFinishTime;
        std::string Uid;
        std::string UUID;
        std::string Value;
    } alarm_struct_t;

    typedef struct OperationStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        std::string Status;
    } operation_struct_t;

    typedef struct PushStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        std::string Uid;
        std::string Value;
    } push_struct_t;

    typedef struct ProgixStruct
    {
        mqtt::topic_e Topic;
        uint64_t SourceTimestamp;
        std::string Value;
    } progix_struct_t;

    class JSON
    {
    public:
        JSON() {}
        virtual ~JSON() {}
    public:
        std::string Serialize(const jarvis_struct_t& _struct);
        std::string Serialize(const remote_controll_struct_t& _struct);
        std::string Serialize(const jarvis_interface_struct_t& _struct);
        size_t Serialize(const fota_status_t& _struct, const size_t size, char output[]);

    public:
        void Serialize(const daq_struct_t& msg, const uint16_t size, char output[]);
        void Serialize(const alarm_struct_t& msg, const uint16_t size, char output[]);
        void Serialize(const operation_struct_t& msg, const uint16_t size, char output[]);
        void Serialize(const progix_struct_t& msg, const uint16_t size, char output[]);
        void Serialize(const push_struct_t& msg, const uint16_t size, char output[]);
        void Serialize(const req_head_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_head_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_vsn_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_mem_t& msg, const uint16_t size, char output[]);
        void Serialize(const resp_status_t& msg, const uint8_t size, char output[]);
        void Serialize(const spear_remote_control_msg_t& msg, const uint8_t size, char output[]);

    public:
        Status Deserialize(const char* payload, JsonDocument* json);
        Status Deserialize(const std::string& payload, JsonDocument* json);
        Status Deserialize(fs::File& file, JsonDocument* json);
    private:
        Status processErrorCode(const DeserializationError& errorCode);
        // Status Deserialize(const spear_msg_t& message, JsonDocument* json);
    
    // private:
    //     std::string serializeJarvisResponse(const jarvis_struct_t& _struct);
    // private:
    //     std::string serializeScautrRemoteControllResponse(const remote_controll_struct_t& _struct);
    // private:
    //     std::string serializeDaqInput(const daq_struct_t& _struct);
    //     std::string serializeDaqOutput(const daq_struct_t& _struct);
    //     std::string serializeDaqParam(const daq_struct_t& _struct);
    // private:
    //     std::string serializeScautrAlarm(const remote_controll_struct_t& _struct);
    //     std::string serializeScautrError(const remote_controll_struct_t& _struct);
    };
}