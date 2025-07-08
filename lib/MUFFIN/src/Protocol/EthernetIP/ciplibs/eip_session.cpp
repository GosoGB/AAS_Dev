#include "eip_session.h"
#include "cip_util.h"

// EtherNet/IP Adaptation of CIP, Chapter 2: Encapsulation Protocol 문서 참조
// Encapsulation command codes
constexpr uint16_t EIP_CMD_REGISTER_SESSION   = 0x0065;
constexpr uint16_t EIP_CMD_UNREGISTER_SESSION = 0x0066;

// Encapsulation header sizes
constexpr size_t EIP_HEADER_SIZE = 24;
constexpr uint16_t EIP_PROTOCOL_VERSION = 0x0001;

EIPSession embeddedEipSession_t;
EIPSession link1EipSession_t;

bool eipInit(EIPSession& session, IPAddress ip, uint16_t port) {
    session.targetIP = ip;
    session.targetPort = port;

    session.client->stop();
    delay(100); // ensure previous session is fully closed

    if (session.client->connect(ip, port)) {
        session.connected = true;
        return true;
    }
    return false;
}

bool registerSession(EIPSession& session) {
    std::vector<uint8_t> packet = {
        // Encapsulation Header (24 bytes)
        0x65, 0x00,             // Command: RegisterSession
        0x04, 0x00,             // Length: 4
        0x00, 0x00, 0x00, 0x00, // Session Handle
        0x00, 0x00, 0x00, 0x00, // Status
        0x00, 0x00, 0x00, 0x00, // Sender Context (1st half)
        0x00, 0x00, 0x00, 0x00, // Sender Context (2nd half)
        0x00, 0x00, 0x00, 0x00, // Options (4 bytes)

        // Command-specific Data (4 bytes)
        0x01, 0x00,             // Protocol Version = 1
        0x00, 0x00              // Options Flags = 0
    };

    if (session.client->write(packet.data(), packet.size()) != packet.size()) {
        Serial.println("[RegisterSession] 요청 전송 실패");
        return false;
    }

    // 1. 먼저 Encapsulation Header (24바이트) 수신
    unsigned long start = millis();
    uint8_t header[EIP_HEADER_SIZE] = {0};
    int totalRead = 0;

    while (totalRead < EIP_HEADER_SIZE && millis() - start < 1000) {
        if (session.client->available()) {
            int r = session.client->read(header + totalRead, EIP_HEADER_SIZE - totalRead);
            if (r > 0) {
                totalRead += r;
            } else if (r < 0) {
                Serial.println("[RegisterSession] 수신 중 오류 발생");
                return false;
            }
        }
    }

    if (totalRead < EIP_HEADER_SIZE) {
        Serial.println("[RegisterSession] 응답 수신 실패 (헤더 부족)");
        return false;
    }

    // 수신된 데이터의 RAW "65 00"
    // header[0] = LSB (예: 0x65)
    // header[1] = MSB (예: 0x00)
    uint16_t command = (header[1] << 8) | header[0];

    if (command != 0x0065) {
        Serial.printf("[RegisterSession] 올바르지 않은 응답 커맨드: 0x%04X\n", command);
        return false;
    }

    // 2. Length 필드에서 payload 길이 확인 -> 수신 확인 "04 00", endian 처리
    uint16_t payloadLen = (header[3] << 8) | header[2];

    // 3. payload 읽기
    std::vector<uint8_t> response;
    response.insert(response.end(), header, header + EIP_HEADER_SIZE);

    totalRead = 0;
    start = millis();

    while (totalRead < payloadLen && millis() - start < 1000) {
        if (session.client->available()) {
            uint8_t buf[256];
            int toRead = std::min((int)sizeof(buf), (int)(payloadLen - totalRead));
            int r = session.client->read(buf, toRead);
            if (r > 0) {
                response.insert(response.end(), buf, buf + r);
                totalRead += r;
            } else if (r < 0) {
                Serial.println("[RegisterSession] payload 수신 중 오류");
                return false;
            }
        }
    }

    if (totalRead < payloadLen) {
        Serial.printf("[RegisterSession] payload 수신 실패 (부족) → 수신: %u / Encapsulation Header length: %u\n", totalRead, payloadLen);
        return false;
    }

    // 수신데이터 "01 00 13 00" -> 0x00130001
    // Extract Session Handle ( Little Endian 처리)
    session.sessionHandle = (uint32_t)(
        (response[7] << 24) |
        (response[6] << 16) |
        (response[5] << 8)  |
        (response[4])
    );

    // Serial.print("[RegisterSession] 받은 sessionHandle: ");
    // Serial.println(session.sessionHandle, HEX);

    return (session.sessionHandle != 0);
}

bool unregisterSession(EIPSession& session) {
    if (!session.connected || session.sessionHandle == 0) return false;

    std::vector<uint8_t> packet = {
        0x66, 0x00,     // Command: UnregisterSession (0x0066 little-endian)
        0x00, 0x00,     // Length = 0

        // Session Handle (little-endian 4바이트)
        static_cast<uint8_t>(session.sessionHandle & 0xFF),
        static_cast<uint8_t>((session.sessionHandle >> 8) & 0xFF),
        static_cast<uint8_t>((session.sessionHandle >> 16) & 0xFF),
        static_cast<uint8_t>((session.sessionHandle >> 24) & 0xFF),

        // Status (4B) = 0
        0x00, 0x00, 0x00, 0x00,

        // Sender Context (8B) = 0
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        // Options (4B) = 0
        0x00, 0x00, 0x00, 0x00
    };


    session.client->write(packet.data(), packet.size());
    return true;
}

void eipClose(EIPSession& session) {
    unregisterSession(session); // try clean close
    session.client->stop();

    session.connected = false;
    session.sessionHandle = 0;
    session.connectionID = 0;
}
