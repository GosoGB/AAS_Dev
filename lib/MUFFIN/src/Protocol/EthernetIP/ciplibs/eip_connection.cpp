#include "eip_connection.h"
#include "cip_util.h"

bool forwardOpen(EIPSession& session) {
    std::vector<uint8_t> service;

    // ForwardOpen request 기본 구조 (간소화된 버전)
    service.insert(service.end(), {
        0x54,  // Service: Forward Open
        0x02,  // Path size
        0x20, 0x06,  // Class: Connection Manager
        0x24, 0x01   // Instance: 1
    });

    // ForwardOpen request data (고정값)
    //  참조 코드
    // https://github.com/nimbuscontrols/EIPScanner/blob/master/src/cip/connectionManager/ConnectionParameters.h
    // https://github.com/nimbuscontrols/EIPScanner/blob/master/src/cip/connectionManager/ForwardOpenRequest.cpp
    std::vector<uint8_t> data = {
        0x00, 0x00, 0x00, 0x00,   // Connection ID (O->T)
        0x01, 0x00,               // Connection Serial Number
        0x34, 0x12,               // Vendor ID
        0x78, 0x56,               // Originator Serial Number
        0x03,                    // Timeout Multiplier
        0x00, 0x00,              // Reserved
        0xF4, 0x01, 0x00, 0x00,  // O->T RPI (500ms)
        0x43, 0x00,              // O->T Parameters
        0xF4, 0x01, 0x00, 0x00,  // T->O RPI
        0x43, 0x00,              // T->O Parameters
        0xA3,                    // Transport type + trigger
        0x02, 0x20, 0x04, 0x24, 0x64, 0x2C, 0x01 // Connection Path
    };

    service.insert(service.end(), data.begin(), data.end());

    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, service, response)) {
        Serial.println("[forwardOpen] Failed to send request");
        return false;
    }

    // Serial.println("[forwardOpen] Response:");
    // printHex(response);    

    // Encapsulation Header: sessionHandle = offset 4~7
    if (response.size() >= 8) {
        uint32_t receivedHandle =
            (response[7] << 24) |
            (response[6] << 16) |
            (response[5] << 8)  |
            (response[4]);

        // 수신된 값이 현재 세션의 sessionHandle과 다르면 오류
        if (receivedHandle != session.sessionHandle) {
            Serial.print("[forwardOpen] 오류: 수신된 sessionHandle 불일치. 받은 값: 0x");
            Serial.print(receivedHandle, HEX);
            Serial.print(" / 송신된 sessionHandle : 0x");
            Serial.println(session.sessionHandle, HEX);
            return false;
        }

        // Serial.print("[forwardOpen] 받은 sessionHandle 확인됨: ");
        // Serial.println(receivedHandle, HEX);
    }

    // 참조 코드
    // https://github.com/EIPStackGroup/OpENer/blob/master/source/src/cip/cipconnectionmanager.c
    // AssembleForwardOpenResponse(..)
    // CIP 응답은 offset 24부터 시작
    int cipStart = 24;
    if (response.size() > cipStart + 10) {
        uint8_t generalStatus = response[cipStart + 2];
        uint8_t additionalStatusCount = response[cipStart + 3];
        int additionalStatusBytes = additionalStatusCount * 2;

        // CIP 응답에서 ConnectionID 위치는 고정 아님 → GeneralStatus 블록 뒤에 위치
        int connIDOffset = cipStart + 4 + additionalStatusBytes;

        if (response.size() >= connIDOffset + 4) {
            session.connectionID =
                response[connIDOffset] |
                (response[connIDOffset + 1] << 8) |
                (response[connIDOffset + 2] << 16) |
                (response[connIDOffset + 3] << 24);

            // Serial.print("[forwardOpen] 받은 connectionID: ");
            // Serial.println(session.connectionID, HEX);
            return true;
        } else {
            Serial.println("[forwardOpen] 응답 내 connectionID 위치 부족");
        }
    } else {
        Serial.println("[forwardOpen] CIP 응답 영역 부족");
    }   

    return false;
}

bool forwardClose(EIPSession& session) {
    std::vector<uint8_t> service = {
        0x4E, 0x02,       // Service: Forward Close, Path size
        0x20, 0x06,       // Class: Connection Manager
        0x24, 0x01        // Instance: 1
    };

    // ForwardClose data (고정값)
    // 참조 코드
    // https://github.com/nimbuscontrols/EIPScanner/blob/master/src/cip/connectionManager/ForwardCloseRequest.cpp
    std::vector<uint8_t> data = {
        0x01, 0x00,               // Connection Serial Number
        0x34, 0x12,               // Vendor ID
        0x78, 0x56,               // Originator Serial Number
        0x00, 0x00                // Reserved
    };

    service.insert(service.end(), data.begin(), data.end());

    std::vector<uint8_t> response;
    if (!sendEncapsulationPacket(session, service, response)) {
        Serial.println("[forwardClose] Failed to send request");
        return false;
    }

    return true;
}