#ifndef CIP_MSR_H
#define CIP_MSR_H

#include <vector>
#include <string>
#include "cip_types.h"
#include "eip_types.h"

// 다중 TAG 읽기 요청 (Multiple Service Request - 0x0A)
bool readTagsMSR(EIPSession& session, const std::vector<std::string>& tagNames, std::vector<cip_data_t>& outValues);

// 다중 TAG 쓰기 요청 (MSR 기반 Write), 개별 응답
bool writeTagsMSR(EIPSession& session, const std::vector<std::string>& tagNames, const std::vector<std::vector<uint8_t>>& values, const std::vector<uint16_t>& dataTypes, std::vector<cip_data_t>& results);

#endif