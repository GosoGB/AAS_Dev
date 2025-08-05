#if defined(MT11)

#ifndef CIP_CLIENT_H
#define CIP_CLIENT_H

#include <vector>
#include <string>
#include "cip_types.h"
#include "eip_types.h"
#include "Common/Allocator/psramAllocator.h"

// Logical Segment로 값 읽기
bool readLogicalValue(EIPSession& session, uint8_t classId, uint8_t instanceId, uint8_t attributeId, uint16_t dataType, cip_data_t& outResult);

// Logical Segment로 값 쓰기
bool writeLogicalValue(EIPSession& session, uint8_t classId, uint8_t instanceId, uint8_t attributeId, uint16_t dataType, const psramVector<uint8_t>& data, cip_data_t& outResult);

#endif

#endif