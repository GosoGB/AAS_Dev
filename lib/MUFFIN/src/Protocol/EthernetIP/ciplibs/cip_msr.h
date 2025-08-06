#if defined(MT11)

#ifndef CIP_MSR_H
#define CIP_MSR_H

#include <vector>
#include <string>
#include "cip_types.h"
#include "eip_types.h"
#include "Common/PSRAM.hpp"

// 다중 TAG 읽기 요청 (Multiple Service Request - 0x0A)
bool readTagsMSR(EIPSession& session, const muffin::psram::vector<std::string>& tagNames, muffin::psram::vector<cip_data_t>& outValues);

// 다중 TAG 쓰기 요청 (MSR 기반 Write), 개별 응답
bool writeTagsMSR(EIPSession& session, const muffin::psram::vector<std::string>& tagNames, const muffin::psram::vector<muffin::psram::vector<uint8_t>>& values, const muffin::psram::vector<uint16_t>& dataTypes, muffin::psram::vector<cip_data_t>& results);

#endif

#endif