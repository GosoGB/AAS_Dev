#if defined(MT11)

#include "cip_msr.h"
#include "cip_path.h"
#include "cip_util.h"
#include "Common/Time/TimeUtils.h"

// Rockwell Automation Publication 1756-PM020H-EN-P - March 2022 - page 31
bool readTagsMSR(EIPSession& session, const std::vector<std::string>& tagNames, std::vector<cip_data_t>& outValues) {

    constexpr size_t CIP_OFFSET = 40;                                   // Encapsulation Header + RR Data

    std::vector<uint8_t> message;

    // [0] Service Code: Multiple Service Request
    message.push_back(0x0A);
    // [1] Request Path Size = 2 words (4 bytes)
    message.push_back(0x02);  // pathSize = 2 words

    // [2~5] Request Path: Class = 0x02, Instance = 0x01 (Message Router)
    message.push_back(0x20);  
    message.push_back(0x02);  
    message.push_back(0x24);  
    message.push_back(0x01);  

    // [6~7] Number of Services contained in this request 
    size_t requestCount = tagNames.size();
    message.push_back(static_cast<uint8_t>(requestCount & 0xFF));        // LSB
    message.push_back(static_cast<uint8_t>((requestCount >> 8) & 0xFF)); // MSB

    // Request Data 기준 위치
    size_t offsetTableStart = message.size();
    for (size_t i = 0; i < requestCount; ++i) {
        message.push_back(0x00);  // Offset LSB
        message.push_back(0x00);  // Offset MSB
    }
    
    std::vector<uint8_t> embeddedRequests;
    size_t currentOffset = (requestCount * 2) + 2; // Number of Services contained in this request 이후 위치

    // Build and append each embedded request
    for (size_t i = 0; i < requestCount; ++i) {
        std::vector<uint8_t> request;

        // [0] Embedded Service Code: Read Tag (0x4C)
        request.push_back(0x4C);

        // [1] Path size (in 16-bit words)
        std::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagNames[i]);
        request.push_back(static_cast<uint8_t>(path.size() / 2));

        // [2~] Path segment
        request.insert(request.end(), path.begin(), path.end());

        // [n~n+1] Element count (2 bytes, LE) — usually 1
        request.push_back(0x01);  // LSB
        request.push_back(0x00);  // MSB

        // Write offset into offset table
        size_t offsetPos =  offsetTableStart + i * 2;
        message[offsetPos] = static_cast<uint8_t>(currentOffset & 0xFF);
        message[offsetPos + 1] = static_cast<uint8_t>((currentOffset >> 8) & 0xFF);

        // Append this request
        embeddedRequests.insert(embeddedRequests.end(), request.begin(), request.end());
        currentOffset += request.size();
    }

    // Append all embedded requests after offset table
    message.insert(message.end(), embeddedRequests.begin(), embeddedRequests.end());

    // Send packet
    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) {
        return false;
    }

    // Parse each response segment from Multiple Service Response (0x8A)
    // Read Tag Fragmented Service Error Codes 1756-pm020_-en-p.pdf page 23
    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("Response too short for CIP payload");
        return false;
    }

    // CIP Service Code
    uint8_t serviceCode = response[CIP_OFFSET];
    // Serial.printf("Service Code: 0x%02X\n", serviceCode);

    // Find 0x8A Service Code ( MSR 응답 코드 )
    if (serviceCode != 0x8A) {
        Serial.printf("Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    // General Status
    uint8_t generalStatus = response[CIP_OFFSET + 2];
    // Serial.printf("General Status: 0x%02X\n", generalStatus);
    
    // 0x1E : Embedded service error. One or more services returned an error within a multiple-service packet service.
    // 참조 : https://support.ptc.com/help/kepware/drivers/en/index.html#page/kepware/drivers/OMRONNJETHERNET/CIP_Error_Codes.html
    if (generalStatus != 0x00 && generalStatus != 0x1E) {   
        Serial.println("오류 상태, 데이터 없음");
        return false;
    }

    // Additional Status Size
    uint8_t additionalStatusSize = response[CIP_OFFSET + 3];
    size_t dataStart = CIP_OFFSET + 4 + additionalStatusSize * 2;   // reply data

    if (response.size() < dataStart + 4) {      // 최소 사이즈 체크
        Serial.println("데이터 필드 없음");
        return false;
    }

    uint16_t itemCount = response[dataStart] |
                        (response[dataStart + 1] << 8);

    // Serial.printf("Number of Service Replies (itemCount) = %d\n", itemCount);
    // Serial.printf("dataStart = %zu\n", dataStart);

    // 응답 반복 구간
    size_t offsetListStart = dataStart + 2;
    // Serial.printf("offsetListStart = %zu\n", offsetListStart);

    size_t base = dataStart; // CIP_OFFSET;
    // Serial.printf("base = %zu\n", base);

    outValues.clear();

    for (int i = 0; i < itemCount; ++i) {
        uint16_t offset = response[offsetListStart + i * 2] |
                        (response[offsetListStart + i * 2 + 1] << 8);
        size_t start = base + offset;

        size_t nextOffset = (i + 1 < itemCount)
                        ? (response[offsetListStart + (i + 1) * 2] |
                            (response[offsetListStart + (i + 1) * 2 + 1] << 8))
                        : (response.size() - base);
        size_t end = base + nextOffset;

        // Serial.printf("\n응답[%d]: offset=0x%04X, start=%zu, end=%zu\n", i, offset, start, end);

        if (start >= response.size() || end > response.size() || start >= end) {
            Serial.printf("범위 오류: start=%zu, end=%zu, 전체 크기=%zu\n", start, end, response.size());
            continue;
        }

        std::vector<uint8_t> oneResponse(response.begin() + start, response.begin() + end);

        // 전체 응답 바이트 출력 - 디버깅 코드
        // Serial.print("전체 응답: ");
        // for (auto b : oneResponse) Serial.printf("%02X ", b);
        // Serial.println();

        if (oneResponse.size() < 6) {
            Serial.println("응답 너무 짧음 (6바이트 미만)");
            continue;
        }

        uint8_t generalStatus = oneResponse[2];
        uint8_t extStatusSize = oneResponse[3];

        // Serial.printf("GeneralStatus=0x%02X, ExtStatusSize=%d\n", generalStatus, extStatusSize);

        if (generalStatus != 0x00) {
            Serial.printf("GeneralStatus 오류, 응답 스킵\n");
            continue;
        }

        size_t typePos = 4 + extStatusSize * 2;
        if (typePos + 1 >= oneResponse.size()) {
            Serial.println("TagType 위치 초과");
            continue;
        }

        uint16_t rawType = oneResponse[typePos] | (oneResponse[typePos + 1] << 8);
        int valueSize = cipDataTypeSizeFromRaw(rawType);

        // Serial.printf("TagType=0x%04X (%d 바이트)\n", rawType, valueSize);

        size_t valuePos = typePos + 2;
        std::vector<uint8_t> value;

        if (rawType == 0x02A0) 
        {
        // STRING 구조체 처리
            valuePos +=2;
            if (valuePos + 4 > oneResponse.size()) 
            {
                Serial.println("STRING 길이 필드 부족");
                continue;
            }

            uint32_t strLen = oneResponse[valuePos] |
                            (oneResponse[valuePos + 1] << 8) |
                            (oneResponse[valuePos + 2] << 16) |
                            (oneResponse[valuePos + 3] << 24);

            // Serial.printf("STRING 길이: %u\n", strLen);

            if (strLen > 82 || valuePos + 4 + strLen > oneResponse.size()) 
            {
                Serial.println("STRING 길이 초과 또는 부족");
                continue;
            }

            // 유효한 길이만큼 value에 복사
            value = std::vector<uint8_t>(oneResponse.begin() + valuePos + 4,
                                        oneResponse.begin() + valuePos + 4 + strLen);
        } 
        else 
        {
            if (valueSize <= 0 || valuePos + valueSize > oneResponse.size()) 
            {
                Serial.printf("값 크기 오류 또는 미지원 타입\n");
                continue;
            }
            value = std::vector<uint8_t>(oneResponse.begin() + valuePos,
                                        oneResponse.begin() + valuePos + valueSize);
        }                   



        // Value
        // std::vector<uint8_t> value(oneResponse.begin() + valuePos, oneResponse.begin() + valuePos + valueSize);

        // Serial.print("Value = ");
        // for (auto b : value) Serial.printf("%02X ", b);
        // Serial.println();

        // cip_data_t에 담기
        cip_data_t data;
        data.Code = generalStatus;
        data.ExtCode = extStatusSize;
        data.Timestamp = muffin::GetTimestampInMillis();
        data.DataType = static_cast<CipDataType>(rawType);

        // 값 디코딩
        if (!decodeCipValue(data.DataType, valuePos, oneResponse, data.Value, data.RawData))
        {
            Serial.printf("[readTag] Unknown or unsupported DataType: 0x%04X\n", rawType);
            data.Code = 0xFF; // 디코딩 실패 표시 (선택)
        }

        // 응답 배열 추가
        outValues.push_back(data);
    }
    return true;
}

/* tag 별 응답 */
bool writeTagsMSR(EIPSession& session,
                  const std::vector<std::string>& tagNames,
                  const std::vector<std::vector<uint8_t>>& values,
                  const std::vector<uint16_t>& dataTypes,  // 20250620 추가됨
                  std::vector<cip_data_t>& results ) {

    constexpr size_t CIP_OFFSET = 40;                                   // Encapsulation Header + RR Data                    

    // 파라미터 수 검증
    if (tagNames.size() != values.size() || tagNames.size() != dataTypes.size()) return false;

    std::vector<uint8_t> message;

    // [0] Service Code: Multiple Service Request
    message.push_back(0x0A);
    // [1] Request Path Size = 2 words (4 bytes)
    message.push_back(0x02);  // pathSize = 2 words

    // [2~5] Request Path: Class = 0x02, Instance = 0x01 (Message Router)
    message.push_back(0x20);  
    message.push_back(0x02);  
    message.push_back(0x24);  
    message.push_back(0x01);  

    // [6~7] Number of Services contained in this request 
    size_t requestCount = tagNames.size();
    message.push_back(static_cast<uint8_t>(requestCount & 0xFF));        // LSB
    message.push_back(static_cast<uint8_t>((requestCount >> 8) & 0xFF)); // MSB

    // Request Data 기준 위치
    size_t offsetTableStart = message.size();
    for (size_t i = 0; i < requestCount; ++i) {
        message.push_back(0x00);  // Offset LSB
        message.push_back(0x00);  // Offset MSB
    }

    std::vector<uint8_t> segmentData;
    //std::vector<uint8_t> embeddedRequests;
    size_t currentOffset = (requestCount * 2) + 2; // Number of Services contained in this request 이후 위치

    /* * 중요 tag는 단일 기준으로, Element count = 1,  array를 지원하지 않음, 이부분은 관리 포인트도 고려해야 하는 사항 */
    for (size_t i = 0; i < tagNames.size(); ++i) {
        std::vector<uint8_t> req = {0x4D}; // Write Tag request (0x4D)

        // Encode tag path
        std::vector<uint8_t> path = encodeTagPathWithMultiIndex(tagNames[i]);
        req.push_back(static_cast<uint8_t>(path.size() / 2)); // Path size (in 16-bit words)
        req.insert(req.end(), path.begin(), path.end());

        // Data Type (from dataTypes[i], Little Endian),  type 처리 가능하도록 수정
        uint16_t type = dataTypes[i];
        req.push_back(static_cast<uint8_t>(type & 0xFF));        // LSB
        req.push_back(static_cast<uint8_t>((type >> 8) & 0xFF)); // MSB
        
        // Element count (assume 4-byte units)
        CipDataType typeEnum = static_cast<CipDataType>(type);
        int typeSize = cipDataTypeSize(typeEnum);

        uint16_t elementCount = 1;  // 방어 코드 : 기본값은 1로 설정, Multiple에서는 Number of embedded requests와 맞춤

        bool valid = true;
        if (typeSize == 0 || values[i].size() % typeSize != 0) {
            Serial.printf("[writeTagsMSR] Invalid data size for tag %s, forcing elementCount = 1\n", tagNames[i].c_str());
            valid = false;
            typeSize = 1;  // 방어 코드 : 0으로 두면 division by zero 위험
        } else {
            elementCount = values[i].size() / typeSize;
        }
        req.push_back(static_cast<uint8_t>(elementCount & 0xFF));       // Element Count LSB
        req.push_back(static_cast<uint8_t>((elementCount >> 8) & 0xFF)); // Element Count MSB

        // Actual data
        req.insert(req.end(), values[i].begin(), values[i].end());

        // Insert offset into Offset Table (정확한 위치 덮어쓰기)
        size_t offsetPos = offsetTableStart + i * 2;
        message[offsetPos] = static_cast<uint8_t>(currentOffset & 0xFF);
        message[offsetPos + 1] = static_cast<uint8_t>((currentOffset >> 8) & 0xFF);

        segmentData.insert(segmentData.end(), req.begin(), req.end());
        currentOffset += req.size();
    }

    message.insert(message.end(), segmentData.begin(), segmentData.end());

    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, message, response)) return false;

    // 응답 해석 : Rockwell Automation Publication 1756-PM020H-EN-P - March 2022, page 32
    // Extended Status Size 는 항상 0이 아니므로 패킷 해석에 주의, 사이즈에 따라 Item count 위치가 변함
    // 응답 서비스 코드가 가변인 경우 해당 코드를 찾아서 처리, 1756-pm020_-en-p.pdf page 31
    if (response.size() < CIP_OFFSET + 4) {
        Serial.println("Response too short for CIP payload");
        return false;
    }

    // CIP Service Code
    uint8_t serviceCode = response[CIP_OFFSET];
    // Serial.printf("Service Code: 0x%02X\n", serviceCode);

    // Find 0x8A Service Code ( MSR 응답 코드 )
    if (serviceCode != 0x8A) {
        Serial.printf("Unexpected Service Code: 0x%02X\n", serviceCode);
        return false;
    }

    // General Status
    uint8_t generalStatus = response[CIP_OFFSET + 2];
    // Serial.printf("General Status: 0x%02X\n", generalStatus);
    
    // 0x1E : Embedded service error. One or more services returned an error within a multiple-service packet service.
    // 참조 : https://support.ptc.com/help/kepware/drivers/en/index.html#page/kepware/drivers/OMRONNJETHERNET/CIP_Error_Codes.html
    if (generalStatus != 0x00 && generalStatus != 0x1E) {   
        Serial.println("오류 상태, 데이터 없음");
        return false;
    }

    // Additional Status Size
    uint8_t additionalStatusSize = response[CIP_OFFSET + 3];
    size_t dataStart = CIP_OFFSET + 4 + additionalStatusSize * 2;   // reply data

    if (response.size() < dataStart + 4) {      // 최소 사이즈 체크
        Serial.println("데이터 필드 없음");
        return false;
    }

    uint16_t itemCount = response[dataStart] |
                        (response[dataStart + 1] << 8);

    // Serial.printf("Number of Service Replies (itemCount) = %d\n", itemCount);
    // Serial.printf("dataStart = %zu\n", dataStart);

    // 응답 반복 구간
    size_t offsetListStart = dataStart + 2;
    // Serial.printf("offsetListStart = %zu\n", offsetListStart);

    size_t base = dataStart; //CIP_OFFSET;
    // Serial.printf("base = %zu\n", base);   

    results.clear();

    // 응답된 서비스 개수(itemCount)만큼 반복하며 각 응답 세그먼트 처리
    for (size_t i = 0; i < itemCount; ++i) {
        uint16_t offset = response[offsetListStart + i * 2] |
                        (response[offsetListStart + i * 2 + 1] << 8);
        size_t start = base + offset;

        size_t nextOffset = (i + 1 < itemCount)
                        ? (response[offsetListStart + (i + 1) * 2] |
                            (response[offsetListStart + (i + 1) * 2 + 1] << 8))
                        : (response.size() - base);
        size_t end = base + nextOffset;

        // Serial.printf("응답[%d]: offset=0x%04X, start=%zu, end=%zu\n", i, offset, start, end);

        if (start >= response.size() || end > response.size() || start >= end) {
            Serial.printf("범위 오류: start=%zu, end=%zu, 전체 크기=%zu\n", start, end, response.size());
            continue;
        }

        std::vector<uint8_t> oneResponse(response.begin() + start, response.begin() + end);

        if (oneResponse.size() < 4) {
            Serial.println("응답 너무 짧음");
            continue;
        }

        uint8_t generalStatus = oneResponse[2];
        uint8_t extStatusSize = oneResponse[3];

        // cip_data_t 생성
        cip_data_t data;
        data.Code = generalStatus;
        data.Timestamp = muffin::GetTimestampInMillis();
        data.DataType = CipDataType::BOOL;      // Write는 데이터 타입이 없으므로 placeholder
        data.Value.BOOL = (generalStatus == 0x00);

        results.push_back(data);
    }

    return true;
}

#endif