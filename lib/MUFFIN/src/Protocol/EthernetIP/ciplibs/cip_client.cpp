#include "cip_types.h"
#include "cip_path.h"
#include "cip_util.h"
#include "cip_client.h"
#include "Common/Time/TimeUtils.h"

// 읽기 함수 (데이터 타입 파라미터화)
bool readLogicalValue( EIPSession& session, uint8_t classId, uint8_t instanceId, uint8_t attributeId, uint16_t dataType, cip_data_t& outResult)
{
    constexpr size_t CIP_OFFSET = 40;

    std::vector<uint8_t> path = buildLogicalPath(classId, instanceId, attributeId);

    std::vector<uint8_t> message;
    message.push_back(0x4C); // Read Tag
    message.push_back(static_cast<uint8_t>(path.size() / 2));
    message.insert(message.end(), path.begin(), path.end());

    // Element Count =1
    message.push_back(0x01);
    message.push_back(0x00);

    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        outResult.Code = 0xFF;
        return false;
    }

    if (response.size() < CIP_OFFSET + 4) {
        outResult.Code = 0xFF;
        return false;
    }

    uint8_t serviceCode = response[CIP_OFFSET];
    if (serviceCode != 0xCC) {
        outResult.Code = 0xFF;
        return false;
    }

    uint8_t generalStatus = response[CIP_OFFSET + 2];
    outResult.Code = generalStatus;
    outResult.ExtCode = response[CIP_OFFSET + 3];
    outResult.Timestamp = muffin::GetTimestampInMillis();
    outResult.DataType = static_cast<CipDataType>(dataType);

    if (generalStatus != 0) {
        return false;
    }

    // Data Type 확인
    size_t dataStart = CIP_OFFSET + 4;
    uint16_t typeRaw = response[dataStart] | (response[dataStart + 1] << 8);
    if (typeRaw != dataType) {
        outResult.Code = 0xFF;
        return false;
    }

    dataStart += 2;

    // 값 파싱 (예제: DINT)
    if (dataType == 0x00C4) { // DINT
        int32_t value = (int32_t)(
            (response[dataStart]) |
            (response[dataStart + 1] << 8) |
            (response[dataStart + 2] << 16) |
            (response[dataStart + 3] << 24)
        );
        outResult.Value.DINT = value;
    } else {
        outResult.Code = 0xFF; // DataType 미지원
        return false;
    }

    return true;
}

// 쓰기 함수 (데이터 타입 파라미터화)
bool writeLogicalValue(EIPSession& session, uint8_t classId, uint8_t instanceId, uint8_t attributeId, uint16_t dataType, const std::vector<uint8_t>& data, cip_data_t& outResult)
{
    constexpr size_t CIP_OFFSET = 40;

    std::vector<uint8_t> path = buildLogicalPath(classId, instanceId, attributeId);

    std::vector<uint8_t> message;
    message.push_back(0x4D); // Write Tag
    message.push_back(static_cast<uint8_t>(path.size() / 2));
    message.insert(message.end(), path.begin(), path.end());

    // Data Type
    message.push_back(static_cast<uint8_t>(dataType & 0xFF));
    message.push_back(static_cast<uint8_t>((dataType >> 8) & 0xFF));

    // Element Count=1
    message.push_back(0x01);
    message.push_back(0x00);

    // Data Payload
    message.insert(message.end(), data.begin(), data.end());

    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        outResult.Code = 0xFF;
        return false;
    }

    if (response.size() < CIP_OFFSET + 4) {
        outResult.Code = 0xFF;
        return false;
    }

    uint8_t serviceCode = response[CIP_OFFSET];
    if (serviceCode != 0xCD) {
        outResult.Code = 0xFF;
        return false;
    }

    outResult.Code = response[CIP_OFFSET + 2];
    outResult.ExtCode = response[CIP_OFFSET + 3];
    outResult.DataType = static_cast<CipDataType>(dataType);
    outResult.Timestamp = muffin::GetTimestampInMillis();

    return (outResult.Code == 0);
}
