/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node와 관련된 데이터 타입들을 선언합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @todo 향후 GUID, ByteString 형식의 Node ID를 구현해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace im {

    constexpr const uint8_t NUM_OF_DATA_TYPE = 12;
    typedef enum class MuffinDataTypeEnum
        : uint8_t
    {
        BOOLEAN  =  0,
        INT8     =  1,
        UINT8    =  2,
        INT16    =  3,
        UINT16   =  4,
        INT32    =  5,
        UINT32   =  6,
        INT64    =  7,
        UINT64   =  8,
        FLOAT32  =  9,
        FLOAT64  = 10,
        STRING   = 11
    } data_type_e;

    /**
     * @todo  //최대 255자 임시용!!!!!!!!! @김주성
     * 
     */
    typedef struct MuffinStringType
    {
        size_t Length;
        char Data[256]; 
    } string_t;

    typedef union VariableDataValueUnion
    {
        bool Boolean;
        int8_t Int8;
        uint8_t UInt8;
        int16_t Int16;
        uint16_t UInt16;
        int32_t Int32;
        uint32_t UInt32;
        int64_t Int64;
        float Float32;
        double Float64;
        uint64_t UInt64;
        string_t String;
    } var_value_u;

    typedef struct PolledDataType
    {
        Status::Code StatusCode;
        jvs::adtp_e AddressType;
        jvs::addr_u Address;
        uint64_t Timestamp;
        jvs::dt_e ValueType;
        var_value_u Value;
    } poll_data_t;

    typedef struct CastedDataType
    {
        jvs::dt_e ValueType;
        var_value_u Value;
    } casted_data_t;

    typedef struct VariableDataType
    {
        Status::Code StatusCode;
        uint64_t Timestamp;
        jvs::dt_e DataType;
        var_value_u Value;
        bool HasValue     : 1;
        bool HasStatus    : 1;
        bool HasTimestamp : 1;
        bool IsEventType  : 1;
        bool HasNewEvent  : 1;
    } var_data_t;



/* For future MUFFIN code base

    typedef union NodePointerUnion
    {
        uint32_t Immediate;
        const NodeID* ID;
        const Base* Node;
    } node_ptr_u;

    typedef struct NodeReferenceTargetType
    {
        node_ptr_u TargetID;
        uint32_t TargetNameHash;
    } ref_target_t;

    /
     * @todo Reference target 개수가 많아진다면 현재 사용 중인 
     *       array 자료구조 보다는 tree 자료구조가 우월합니다.
     *       대략 TargetSize > 8 을 기준으로 보면 됩니다.
     /
    typedef union NodeReferenceKind
    {
        ref_target_t* Targets;
        size_t TargetSize;
        uint8_t ReferenceTypeIndex;
        bool IsInverse;
    } ref_kind_t;
*/
}}