#if defined(MT11)

#ifndef CIP_SINGLE_H
#define CIP_SINGLE_H

#include <vector>
#include <string>
#include "cip_types.h"
#include "eip_types.h"

// Service 코드 참고
// https://docs.pycomm3.dev/en/latest/cip_reference.html

// 단일 TAG 읽기 요청 (0x4C)
bool readTagIndex(EIPSession& session, const std::string& tagName, cip_data_t& outData);
bool readTag(EIPSession& session, const std::string& tagName, cip_data_t& outData);
// Array TAG 읽기 요청 (0x4C)
bool readTagExt(EIPSession& session, const std::string& tagName, uint32_t elementIndex, uint16_t elementCount, std::vector<cip_data_t>& outData );

// 단일 TAG 쓰기 요청 (0x4D)
bool writeTag( EIPSession& session, const std::string& tagName, const std::vector<uint8_t>& data, uint16_t dataType, cip_data_t& outResult );
// Array TAG 쓰기 요청 (0x4D)
bool writeTagExt(EIPSession& session, const std::string& tagName, const std::vector<uint8_t>& data, uint16_t dataType, uint16_t elementCount, cip_data_t& outResult);

#endif

#endif