#ifndef CIP_UTIL_H
#define CIP_UTIL_H

#include <vector>
#include <string>
#include <Arduino.h>
#include <IPAddress.h>
#include <Network/Ethernet/W5500/EthernetClient.h>
#include "cip_types.h"
#include "eip_types.h"

// 디버깅용 HEX 출력
void printHex(const std::vector<uint8_t>& data);

// CIP 데이터 값을 DataType에 따라 출력
void printCipData(const cip_data_t& d);

// CIP 상태코드를 문자열로 변환
std::string getCIPStatusDescription(uint8_t statusCode);

// 데이터 타입의 바이트 크기
int cipDataTypeSize(CipDataType type);

//  Tag Type Value로 데이터 타입의 바이트 크기
int cipDataTypeSizeFromRaw(uint16_t rawType);

//  dataType 디코딩
bool decodeCipValue( CipDataType dataType, size_t dataStart, const std::vector<uint8_t>& response, cip_value_u& outValue, std::vector<uint8_t>& outputRawData );

// 세션 연결 상태 확인
bool checkConnection(muffin::w5500::EthernetClient& client);

// 캡슐화 전송 함수 (SendRRData)
bool sendEncapsulationPacket(EIPSession& session, const std::vector<uint8_t>& serviceData, std::vector<uint8_t>& response);

#endif
