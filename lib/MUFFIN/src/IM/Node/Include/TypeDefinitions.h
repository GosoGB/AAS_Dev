/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node와 관련된 데이터 타입들을 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @todo 향후 GUID, ByteString 형식의 Node ID를 구현해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>



namespace muffin { namespace im {




/* For future MUFFIN code base
    typedef struct MuffinStringType
    {
        size_t Length;
        uint8_t* Data;
    } string_t;

    typedef enum class NodeIdTypeEnum
        : uint8_t
    {
        NUMERIC     = 0,
        STRING      = 3
        // GUID        = 4,
        // BYTE_STRING = 5
    } node_id_type_e;

    typedef union NodeIdentifierUnion
    {
        uint32_t Numeric;
        string_t String;
        // GUID GUID;
        // ByteString ByteString;
    } node_id_u;

    typedef enum class NodeClassTypeEnum
        : uint8_t
    {
        UNSPECIFIED = 0,
        OBJECT = 1,
        VARIABLE = 2,
        METHOD = 4,
        OBJECT_TYPE = 8,
        VARIABLE_TYPE = 16,
        REFERENCE_TYPE = 32,
        DATA_TYPE = 64,
        VIEW = 128
    } class_type_e;

    typedef struct QualifiedNodeNameType
    {
        uint16_t NamespaceIndex;
        string_t Name;
    } qualified_name_t;

    typedef struct LocalizedTextType{
        string_t Local;
        string_t Text;
    } local_txt_t;

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