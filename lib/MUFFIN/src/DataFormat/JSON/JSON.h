/**
 * @file JSON.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 선언합니다.
 * 
 * @date 2024-09-27
 * @version 1.0.0
 * 
 * @todo 인코딩 함수를 추가로 구현해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <Protocol/MQTT/Include/TypeDefinitions.h>
#include "Protocol/SPEAR/Include/TypeDefinitions.h"

#include <vector>

#include "Common/Status.h"


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
        std::string Name;
        std::string Uid;
        std::string Unit;
        std::string Value;
    } daq_struct_t;

    typedef struct AlarmStruct
    {
        mqtt::topic_e Topic;
        std::string AlarmType;
        uint64_t AlarmStartTime;
        int64_t AlarmFinishTime;
        std::string Name;
        std::string Uid;
        std::string UUID;
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
        std::string Name;
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
        std::string Serialize(jarvis_struct_t& _struct);
        std::string Serialize(const remote_controll_struct_t& _struct);
        std::string Serialize(const daq_struct_t& _struct);
        std::string Serialize(const alarm_struct_t& _struct);
        std::string Serialize(const operation_struct_t& _struct);
        std::string Serialize(const push_struct_t& _struct);
        std::string Serialize(const progix_struct_t& _struct);
        size_t Serialize(const fota_status_t& _struct, const size_t size, char output[]);

    public:
        void Serialize(const req_head_t& msg, const uint8_t size, char output[]);
        void Serialize(const req_start_head_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_head_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_vsn_t& msg, const uint8_t size, char output[]);
        void Serialize(const resp_mem_t& msg, const uint16_t size, char output[]);
        void Serialize(const resp_status_t& msg, const uint8_t size, char output[]);
        void Serialize(const spear_remote_control_msg_t& msg, const uint8_t size, char output[]);

    public:
        Status Deserialize(const std::string& payload, JsonDocument* json);
        Status Deserialize(const spear_msg_t& message, JsonDocument* json);
    
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