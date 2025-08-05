#if defined(MT11)

#ifndef CIP_MSR_H
#define CIP_MSR_H

#include <vector>
#include <string>
#include "cip_types.h"
#include "eip_types.h"
#include "Common/Allocator/psramAllocator.h"

// 다중 TAG 읽기 요청 (Multiple Service Request - 0x0A)
bool readTagsMSR(EIPSession& session, const psramVector<std::string>& tagNames, psramVector<cip_data_t>& outValues);

// 다중 TAG 쓰기 요청 (MSR 기반 Write), 개별 응답
bool writeTagsMSR(EIPSession& session, const psramVector<std::string>& tagNames, const psramVector<psramVector<uint8_t>>& values, const psramVector<uint16_t>& dataTypes, psramVector<cip_data_t>& results);

#endif

#endif