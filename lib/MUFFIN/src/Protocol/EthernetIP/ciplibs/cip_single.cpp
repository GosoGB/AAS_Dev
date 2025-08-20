#if defined(MT11)

#include "cip_single.h"
#include "cip_path.h"
#include "cip_util.h"
#include "cip_types.h"
#include "Common/Time/TimeUtils.h"

bool readTagIndex(EIPSession& session, const std::string& tagName, cip_data_t& outData)
{
    constexpr size_t CIP_OFFSET = 40;                                   // Encapsulation Header + RR Data

    muffin::psram::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagName);

    muffin::psram::vector<uint8_t> message;
    message.push_back(0x4C); // Service Code: Read Tag
    message.push_back(static_cast<uint8_t>(path.size() / 2)); // Path Size (WORD)

    message.insert(message.end(), path.begin(), path.end());

    message.push_back(0x01); // Element Count LSB
    message.push_back(0x00); // Element Count MSB

    // Serial.printf("[readTag] Sending Read Tag request: %s\n", tagName.c_str());
    // Serial.printf("[readTag] Path size: %zu bytes\n", path.size());
    // Serial.print("[readTag] Encoded CIP message: ");
    // for (auto b : message)
    //     Serial.printf("%02X ", b);
    // Serial.println();

    muffin::psram::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        Serial.println("[readTag] Failed to send encapsulation request");
        return false;
    }
    
    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("Response too short for CIP payload");
        return false;
    }

    // CIP Service Code
    uint8_t serviceCode = response[CIP_OFFSET];
    // Serial.printf("Service Code: 0x%02X\n", serviceCode);

    // Find 0xCC Service Code 
    if (serviceCode != 0xCC) {
        Serial.printf("Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    // General Status
    uint8_t generalStatus = response[CIP_OFFSET + 2];
    uint8_t extendedStatus = response[CIP_OFFSET + 3];

    outData.Code = generalStatus;
    outData.ExtCode = extendedStatus;
    outData.Timestamp = muffin::GetTimestampInMillis();

    if (generalStatus != 0) {
        Serial.printf("[readTag] CIP Error: General Status=0x%02X\n", generalStatus);
        return false;
    }

    // Data Type
    size_t dataStart = CIP_OFFSET + 4;
    if (response.size() < dataStart + 2) {
        Serial.println("[readTag] Response too short for DataType");
        return false;
    }

    uint16_t typeRaw = response[dataStart] | (response[dataStart + 1] << 8);
    CipDataType dataType = static_cast<CipDataType>(typeRaw);
    outData.DataType = dataType;
    dataStart += 2;

    // Serial.printf("[readTag] Data Type: 0x%04X\n", typeRaw);

    // 데이터 값 처리
    if (!decodeCipValue(dataType, dataStart, response, outData.Value, outData.RawData))
    {
        Serial.printf("[readTag] Unknown DataType: 0x%04X\n", typeRaw);
        return false;
    }
    return true;
}

// 코드 참고
// Rockwell Automation Publication 1756-PM020H-EN-P - March 2022 - page 31
bool readTag(EIPSession& session, const std::string& tagName, cip_data_t& outData)
{
    constexpr size_t CIP_OFFSET = 40;                                   // Encapsulation Header + RR Data

    muffin::psram::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagName);

    muffin::psram::vector<uint8_t> message;
    message.push_back(0x4C); // Service Code: Read Tag
    message.push_back(static_cast<uint8_t>(path.size() / 2)); // Path Size (WORD)

    message.insert(message.end(), path.begin(), path.end());

    message.push_back(0x01); // Element Count LSB
    message.push_back(0x00); // Element Count MSB

    // Serial.printf("[readTag] Sending Read Tag request: %s\n", tagName.c_str());
    // Serial.printf("[readTag] Path size: %zu bytes\n", path.size());
    // Serial.print("[readTag] Encoded CIP message: ");
    // for (auto b : message)
    //     Serial.printf("%02X ", b);
    // Serial.println();

    muffin::psram::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        Serial.println("[readTag] Failed to send encapsulation request");
        return false;
    }
    
    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("Response too short for CIP payload");
        return false;
    }

    // CIP Service Code
    uint8_t serviceCode = response[CIP_OFFSET];
    // Serial.printf("Service Code: 0x%02X\n", serviceCode);

    // Find 0xCC Service Code 
    if (serviceCode != 0xCC) {
        Serial.printf("Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    // General Status
    uint8_t generalStatus = response[CIP_OFFSET + 2];
    uint8_t extendedStatus = response[CIP_OFFSET + 3];

    outData.Code = generalStatus;
    outData.ExtCode = extendedStatus;
    outData.Timestamp = muffin::GetTimestampInMillis();

    if (generalStatus != 0) {
        Serial.printf("[readTag] CIP Error: General Status=0x%02X\n", generalStatus);
        return false;
    }

    // Data Type
    size_t dataStart = CIP_OFFSET + 4;
    if (response.size() < dataStart + 2) {
        Serial.println("[readTag] Response too short for DataType");
        return false;
    }

    uint16_t typeRaw = response[dataStart] | (response[dataStart + 1] << 8);
    CipDataType dataType = static_cast<CipDataType>(typeRaw);
    outData.DataType = dataType;
    dataStart += 2;

    // Serial.printf("[readTag] Data Type: 0x%04X\n", typeRaw);

    // 데이터 값 처리
    if (!decodeCipValue(dataType, dataStart, response, outData.Value, outData.RawData))
    {
        Serial.printf("[readTag] Unknown DataType: 0x%04X\n", typeRaw);
        return false;
    }
    return true;
}

bool readTagExt(EIPSession& session, const std::string& tagName, uint32_t elementIndex, uint16_t elementCount, muffin::psram::vector<cip_data_t>& outData )
{
    constexpr size_t CIP_OFFSET = 40;

    muffin::psram::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagName);

    // Logical Segment 추가, 1756-pm020_-en-p.pdf page 14 , 사용 예제 page 60
    if (elementIndex > 0) {
        muffin::psram::vector<uint8_t> elementSelector = buildElementSelector(elementIndex);
        path.insert(path.end(), elementSelector.begin(), elementSelector.end());
    }

    muffin::psram::vector<uint8_t> message;
    message.push_back(0x4C);                                                // Read Tag Service
    message.push_back(static_cast<uint8_t>(path.size() / 2));               // Path size in words
    message.insert(message.end(), path.begin(), path.end());


    message.push_back(static_cast<uint8_t>(elementCount & 0xFF));           // Element Count LSB
    message.push_back(static_cast<uint8_t>((elementCount >> 8) & 0xFF));    // Element Count MSB

    // Serial.printf("[readTagEx] Sending Read Tag request: %s (Count=%u)\n", tagName.c_str(), elementCount);

    muffin::psram::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        Serial.println("[readTagEx] Failed to send encapsulation request");
        return false;
    }

    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("[readTagEx] Response too short for CIP payload");
        return false;
    }

    uint8_t serviceCode = response[CIP_OFFSET];
    if (serviceCode != 0xCC) {
        Serial.printf("[readTagEx] Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    uint8_t generalStatus = response[CIP_OFFSET + 2];
    uint8_t extendedStatus = response[CIP_OFFSET + 3];

    if (generalStatus != 0) {
        Serial.printf("[readTagEx] CIP Error: General Status=0x%02X\n", generalStatus);
        return false;
    }

    // Data Type
    size_t dataStart = CIP_OFFSET + 4;
    if (response.size() < dataStart + 2) {
        Serial.println("[readTagEx] Response too short for DataType");
        return false;
    }

    uint16_t typeRaw = response[dataStart] | (response[dataStart + 1] << 8);
    CipDataType dataType = static_cast<CipDataType>(typeRaw);
    dataStart += 2;

    // Serial.printf("[readTagEx] Data Type: 0x%04X\n", typeRaw);

    // Element Size
    size_t elementSize = cipDataTypeSize(dataType);
    if (elementSize == 0) {
        Serial.printf("[readTagEx] Unknown DataType size: 0x%04X\n", typeRaw);
        return false;
    }

    size_t expectedSize = elementCount * elementSize;
    if (response.size() < dataStart + expectedSize) {
        Serial.printf("[readTagEx] Response too short for %u elements (need %u bytes)\n", elementCount, expectedSize);
        return false;
    }

    outData.clear();

    // 반복해서 각 Element를 개별 cip_data_t에 채워 넣음
    for (uint16_t i = 0; i < elementCount; ++i) 
    {
        cip_data_t element;
        element.Code = generalStatus;
        element.ExtCode = extendedStatus;
        element.Timestamp = muffin::GetTimestampInMillis();
        element.DataType = dataType;

        size_t elementOffset = dataStart + i * elementSize;

        if (!decodeCipValue(dataType, elementOffset, response, element.Value, element.RawData)) {
            Serial.printf("[readTagEx] Failed to decode element %u\n", i);
            return false;
        }

        outData.push_back(element);
    }

    return true;
}


bool writeTag( EIPSession& session, const std::string& tagName, cip_data_t datum, cip_data_t& outResult )
{
    constexpr size_t CIP_OFFSET = 40;                                   // Encapsulation Header + RR Data
    outResult.Code = 0xFF;              // Unknown error로 초기화

    muffin::psram::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagName);                 // 태그 경로(Symbolic Segment) 생성

    muffin::psram::vector<uint8_t> message;
    message.push_back(0x4D);                                            // Service Code: Write Tag (0x4D)
    message.push_back(static_cast<uint8_t>(path.size() / 2));           // Path size in WORDs (1WORD = 2bytes)
    message.insert(message.end(), path.begin(), path.end());            // 경로 데이터 삽입

    // Data Type
    message.push_back(static_cast<uint8_t>(static_cast<uint16_t>(datum.DataType) & 0xFF));           // Data Type LSB
    message.push_back(static_cast<uint8_t>((static_cast<uint16_t>(datum.DataType) >> 8) & 0xFF));    // Data Type MSB

    CipDataType typeEnum = datum.DataType;
    
    if (static_cast<uint16_t>(datum.DataType) == 0x02A0) 
    { // STRING 처리

        uint32_t strLen = datum.RawData.size();
        const size_t MAX_CIP_STRING_DATA_LEN = 84;

        
        // STRING 구조체: Reserved 2바이트 (보통 0)
        
        message.push_back(datum.Value.STRING.Code[0]);
        message.push_back(datum.Value.STRING.Code[1]);
        
        message.push_back(0x01); // Element Count LSB
        message.push_back(0x00); // Element Count MSB (always 1 for STRING)


        // 길이 (4 bytes, little endian)
        message.push_back(static_cast<uint8_t>(strLen & 0xFF));
        message.push_back(static_cast<uint8_t>((strLen >> 8) & 0xFF));
        message.push_back(static_cast<uint8_t>((strLen >> 16) & 0xFF));
        message.push_back(static_cast<uint8_t>((strLen >> 24) & 0xFF));

        // 문자열 데이터 삽입
        message.insert(message.end(), datum.RawData.begin(), datum.RawData.end());

        for (size_t i = strLen; i < MAX_CIP_STRING_DATA_LEN; ++i) 
        {
            message.push_back(0x00);
        }
    }
    else
    {
        int typeSize = cipDataTypeSize(typeEnum);

        if (typeSize == 0) 
        {
            Serial.printf("[writeTag] Unknown dataType: 0x%04X\n", static_cast<uint16_t>(datum.DataType));
            return false;
        }

        if (datum.RawData.size() % typeSize != 0) 
        {
            Serial.printf("[writeTag] Invalid data size: %u bytes (typeSize=%d)\n", datum.RawData.size(), typeSize);
            return false;
        }

        uint16_t elementCount = static_cast<uint16_t>(datum.RawData.size() / typeSize);
        message.push_back(static_cast<uint8_t>(elementCount & 0xFF));           // Element Count LSB           
        message.push_back(static_cast<uint8_t>((elementCount >> 8) & 0xFF));    // Element Count MSB

        message.insert(message.end(), datum.RawData.begin(), datum.RawData.end()); 
        
        
        Serial.printf("[writeTag] Writing %u element(s) to tag \"%s\" (type=0x%04X)\n",
                          elementCount, tagName.c_str(), static_cast<uint16_t>(datum.DataType));
    }

    // Serial.print("[writeTag] Encoded message: ");
    // for (auto b : message) Serial.printf("%02X ", b);
    // Serial.println();

    // PLC 데이터 전송
    muffin::psram::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        Serial.println("[writeTag] Failed to send encapsulated request");
        return false;
    }

    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("Response too short for CIP payload");
        return false;
    }

    // CIP Service Code
    uint8_t serviceCode = response[CIP_OFFSET];
    Serial.printf("Service Code: 0x%02X\n", serviceCode);

    // Find 0xCD Service Code 
    if (serviceCode != 0xCD) {
        Serial.printf("Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    uint8_t generalStatus = response[CIP_OFFSET + 2];
    uint8_t extendedStatus = response[CIP_OFFSET + 3];

    // Fill cip_data_t
    outResult.Code = generalStatus;
    outResult.ExtCode = extendedStatus;
    outResult.Timestamp = muffin::GetTimestampInMillis();
    outResult.DataType = typeEnum;

    // Write 응답은 값이 없으므로 Value 초기화
    memset(&outResult.Value, 0, sizeof(outResult.Value));

    if (generalStatus != 0x00) {
        Serial.printf("[writeTag] Write failed: General Status=0x%02X\n", generalStatus);
        return false;
    }

    Serial.println("[writeTag] Write successful");
    return true;
}

bool writeTagExt(EIPSession& session, const std::string& tagName, const muffin::psram::vector<uint8_t>& data, uint16_t dataType, uint16_t elementCount, cip_data_t& outResult)
{
    constexpr size_t CIP_OFFSET = 40;   // Encapsulation Header + RR Data
    outResult.Code = 0xFF;              // Unknown error로 초기화

    muffin::psram::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagName);

    muffin::psram::vector<uint8_t> message;
    message.push_back(0x4D); // Write Tag Service
    message.push_back(static_cast<uint8_t>(path.size() / 2)); // Path Size in words
    message.insert(message.end(), path.begin(), path.end());

    // Data Type enum과 타입 크기 계산
    CipDataType typeEnum = static_cast<CipDataType>(dataType);
    int typeSize = cipDataTypeSize(typeEnum);

    if (typeSize == 0) {
        Serial.printf("[writeTagExt] Unknown dataType: 0x%04X\n", dataType);
        return false;
    }

    // 요청에 필요한 총 데이터 크기
    size_t expectedDataSize = static_cast<size_t>(elementCount) * typeSize;
    if (data.size() != expectedDataSize) {
        Serial.printf("[writeTagExt] Invalid data size: got %zu bytes, expected %zu bytes (elements=%u, typeSize=%d)\n",
                      data.size(), expectedDataSize, elementCount, typeSize);
        return false;
    }

    // Data Type
    message.push_back(static_cast<uint8_t>(dataType & 0xFF));
    message.push_back(static_cast<uint8_t>((dataType >> 8) & 0xFF));

    // Element Count (명시적으로 요청에 포함)
    message.push_back(static_cast<uint8_t>(elementCount & 0xFF));
    message.push_back(static_cast<uint8_t>((elementCount >> 8) & 0xFF));

    // Data Payload
    message.insert(message.end(), data.begin(), data.end());

    // Serial.printf("[writeTagExt] Sending Write Tag request: %s (Elements=%u, DataSize=%zu)\n",
    //               tagName.c_str(), elementCount, data.size());

    muffin::psram::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        Serial.println("[writeTagExt] Failed to send encapsulation request");
        return false;
    }

    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("[writeTagExt] Response too short for CIP payload");
        return false;
    }

    uint8_t serviceCode = response[CIP_OFFSET];
    if (serviceCode != 0xCD) { // 0x4D | 0x80 = 0xCD
        Serial.printf("[writeTagExt] Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    uint8_t generalStatus = response[CIP_OFFSET + 2];
    uint8_t extendedStatus = response[CIP_OFFSET + 3];

    outResult.Code = generalStatus;
    outResult.ExtCode = extendedStatus;
    outResult.DataType = typeEnum;
    outResult.Timestamp = muffin::GetTimestampInMillis();

    if (generalStatus != 0) {
        Serial.printf("[writeTagExt] CIP Error: General Status=0x%02X\n", generalStatus);
        return false;
    }

    Serial.println("[writeTagExt] Write successful");
    return true;
}

#endif