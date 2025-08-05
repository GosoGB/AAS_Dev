#if defined(MT11)

// cip_types.h

#pragma once

#include <vector>
#include <cstdint>
#include "IM/Node/Include/TypeDefinitions.h"
#include "Common/Allocator/psramAllocator.h"

// CIP 데이터 타입(Type Codes)
// PUB00123R1_Common-Industrial_Protocol_and_Family_of_CIP_Networks.pdf , page 31
// 1756-pm020_-en-p.pdf 13, Tag type service parameter values used with Logix controllers
// https://www.rockwellautomation.com/en-id/docs/studio-5000-logix-designer/37-00/contents-ditamap/instruction-set/cip-axis-attributes/accessing-attributes/cip-data-types.html
enum class CipDataType : uint16_t {
    // 1-byte types
    BOOL   = 0xC1, // Boolean (1 bit encoded in 1 byte)
    SINT   = 0xC2, // Signed 8-bit integer
    USINT  = 0xC6, // Unsigned 8-bit integer
    BYTE   = 0xD1, // Bit string (8 bits)

    // 2-byte types
    INT    = 0xC3, // Signed 16-bit integer
    UINT   = 0xC7, // Unsigned 16-bit integer
    WORD   = 0xD2, // Bit string (16 bits)

    // 4-byte types
    DINT   = 0xC4, // Signed 32-bit integer
    UDINT  = 0xC8, // Unsigned 32-bit integer
    DWORD  = 0xD3, // Bit string (32 bits)
    REAL   = 0xCA, // 32-bit floating point (IEEE 754)

    // 8-byte types
    LINT   = 0xC5, // Signed 64-bit integer
    ULINT  = 0xC9, // Unsigned 64-bit integer
    LREAL  = 0xCB, // 64-bit floating point (IEEE 754)
    LWORD  = 0xD4, // Bit string (64 bits) // N/A: 일반 CIP 서비스 미지원
    
    // 확인 필요
    STRING = 0x02A0,  // UDT 서비스 코드임
};

typedef struct CipStringType
{
    uint8_t Code[2];
    size_t Length;
    char Data[84]; 
} string_t;

typedef union CipDataValueUnion
{
    // 1-byte
    bool     BOOL;
    int8_t   SINT;
    uint8_t  USINT;
    uint8_t  BYTE; // bit string 8-bit

    // 2-byte
    int16_t  INT;
    uint16_t UINT;
    uint16_t WORD; // bit string 16-bit

    // 4-byte
    int32_t  DINT;
    uint32_t UDINT;
    uint32_t DWORD; // bit string 32-bit
    float    REAL;

    // 8-byte
    int64_t  LINT;
    uint64_t ULINT;
    double   LREAL;
    string_t STRING;
} cip_value_u;

typedef struct CipData
{
    uint16_t Code;                  // 상태 코드
    uint16_t ExtCode;               // Extended Status
    uint64_t Timestamp;             // 수집 시각
    CipDataType  DataType;          // CIP 데이터 타입
    cip_value_u Value;              // 값
    psramVector<uint8_t> RawData;   // Raw Data
} cip_data_t;


// ================================================
// 표준 Class IDs (ODVA + Rockwell CIP 규격)
// https://docs.pycomm3.dev/en/latest/cip_reference.html 참조
// ================================================
enum CipClassId : uint8_t
{
    CIP_CLASS_IDENTITY            = 0x01, // Identity Object
    CIP_CLASS_MESSAGE_ROUTER      = 0x02, // Message Router Object
    CIP_CLASS_ASSEMBLY            = 0x04, // Assembly Object
    CIP_CLASS_CONNECTION          = 0x05, // Connection Object
    CIP_CLASS_CONNECTION_MANAGER  = 0x06  // Connection Manager Object
    // 필요 시 추가
};

// ================================================
// 표준 Instance IDs
// (일부 Object는 Instance 1 고정)
// ================================================
enum CipInstanceId : uint8_t
{
    CIP_INSTANCE_IDENTITY             = 0x01, // Identity Object Instance
    CIP_INSTANCE_MESSAGE_ROUTER       = 0x01, // Message Router Instance
    CIP_INSTANCE_CONNECTION_MANAGER   = 0x01  // Connection Manager Instance
};

// ================================================
// Identity Object Attributes (Class 0x01)
// ================================================
enum CipIdentityAttributeId : uint8_t
{
    CIP_ATTR_VENDOR_ID       = 0x01, // Vendor ID
    CIP_ATTR_DEVICE_TYPE     = 0x02, // Device Type
    CIP_ATTR_PRODUCT_CODE    = 0x03, // Product Code
    CIP_ATTR_REVISION        = 0x04, // Revision (Major.Minor)
    CIP_ATTR_STATUS          = 0x05, // Status
    CIP_ATTR_SERIAL_NUMBER   = 0x06, // Serial Number
    CIP_ATTR_PRODUCT_NAME    = 0x07  // Product Name String
};


#endif