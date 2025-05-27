/**
 * @file DNS.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief DNS 클라이언트 기능을 제공하는 클래스입니다.
 * W5500 이더넷 칩셋을 사용하여 DNS 쿼리를 전송하고, 호스트 이름을 IPv4 주소로 변환합니다.
 * 
 * @details
 * 이 클래스는 W5500 기반의 이더넷 환경에서 DNS 서버와 통신하여 호스트 이름을 IP 주소로 변환하는 기능을 제공합니다.
 * 내부적으로 UDP 소켓을 사용하며, DNS 프로토콜의 표준 쿼리 및 응답 처리를 지원합니다.
 * 
 * @note
 * - UDP 소켓을 외부에서 주입받아 사용합니다.
 * - 현재 IPv4 주소만 지원합니다.
 *
 * @see Socket
 * 
 * @date 2025-05-15
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#pragma once

#include <sys/_stdint.h>

#include "Common/Status.h"
#include "Socket.h"



namespace muffin { namespace w5500 {

    class DNS
    {
    public:
        DNS(Socket& socket) : mSocket(socket) {}
        ~DNS() {}
    public:
        Status Init(const IPAddress dnsServer);
        Status GetHostByName(const char* host, IPAddress* ipv4);
    private:
        /**
         * @brief 점으로 구분된 10진수 문자열 형식의 IPv4 주소를 이진 표현으로 변환합니다.
         *
         * 이 함수는 입력 문자열(예: "192.168.1.1")로 표현된 IPv4 주소를 파싱하여
         * 제공된 IPAddress 구조체에 결과를 저장합니다.
         *
         * @param inputIPv4      점으로 구분된 IPv4 주소가 포함된 널 종료 문자열 포인터입니다.
         * @param outputIPv4     파싱된 주소가 저장될 IPAddress 구조체 포인터입니다.
         * @return Status        변환 성공 또는 실패를 나타내는 Status 코드를 반환합니다.
         */
        Status inet_aton(const char* inputIPv4, IPAddress* outputIPv4);
        
        void createRequest(const char* host, const size_t length, uint8_t* actualLength, uint8_t* output);
        Status waitForResponse();
        Status verifyHeaderUDP(uint16_t* udpPayloadLength);
        Status verifyHeaderDNS();
        size_t parseQuerySection(const size_t length, const uint8_t payload[], std::string* qname, uint16_t* qtype, uint16_t* qclass);
        Status processPayload(const char* host, const size_t payloadLength, IPAddress* resolvedIPv4);
    private:
        Socket& mSocket;
	    uint16_t mRequestId;
        IPAddress mDNSServer;
        const uint16_t mTimeoutMillis = 5000;
        const uint8_t MAX_TRIAL_COUNT = 5;
    private:
        const uint8_t UDP_HEADER_SIZE = 8;
        const uint8_t DNS_HEADER_SIZE = 12;
        const uint8_t TTL_SIZE = 4;
        const uint8_t QUERY_FLAG = 0;
        const uint16_t RESPONSE_FLAG = (1 << 15);
        const uint16_t QUERY_RESPONSE_MASK = (1 << 15);
        const uint8_t OPCODE_STANDARD_QUERY = 0;
        const uint16_t OPCODE_INVERSE_QUERY = (1 << 11);
        const uint16_t OPCODE_STATUS_REQUEST = (2 << 11);
        const uint16_t OPCODE_MASK = (15 << 11);
        const uint16_t AUTHORITATIVE_FLAG = (1 << 10);
        const uint16_t TRUNCATION_FLAG = (1 << 9);
        const uint16_t RECURSION_DESIRED_FLAG = (1 << 8);
        const uint16_t RECURSION_AVAILABLE_FLAG = (1 << 7);
        const uint8_t RESP_NO_ERROR = 0;
        const uint8_t RESP_FORMAT_ERROR = 1;
        const uint8_t RESP_SERVER_FAILURE = 2;
        const uint8_t RESP_NAME_ERROR = 3;
        const uint8_t RESP_NOT_IMPLEMENTED = 4;
        const uint8_t RESP_REFUSED = 5;
        const uint8_t RESP_MASK = 15;
        const uint8_t TYPE_A = 0x01;
        const uint8_t CLASS_IN = 0x01;
        const uint8_t LABEL_COMPRESSION_MASK = 0xC0;
        const uint16_t DNS_PORT = 53;
    };
}}

#endif