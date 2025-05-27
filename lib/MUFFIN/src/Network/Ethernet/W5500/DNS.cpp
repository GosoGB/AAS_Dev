/**
 * @file DNS.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-05-15
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#include <lwip/ip4_addr.h>

#include "Common/Assert.h"
#include "DNS.h"



namespace muffin { namespace w5500 {


    Status DNS::Init(const IPAddress dnsServer)
    {
        ASSERT((uint32_t(dnsServer) != IPADDR_ANY), "INVALID DNS SERVER ADDRESS");
        ASSERT((uint32_t(dnsServer) != IPADDR_NONE), "INVALID DNS SERVER ADDRESS");
        ASSERT((uint32_t(dnsServer) != IPADDR_LOOPBACK), "INVALID DNS SERVER ADDRESS");
        ASSERT((uint32_t(dnsServer) != IPADDR_BROADCAST), "INVALID DNS SERVER ADDRESS");
        ASSERT((mSocket.GetProtocol() == w5500::sock_prtcl_e::UDP), "INVALID SOCKET PROTOCOL");
        
        mRequestId = 0;
        Status ret = mSocket.Open();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET DNS SERVER: %s", ret.c_str());
        }

        mDNSServer = dnsServer;
        return ret;
    }


    Status DNS::GetHostByName(const char* host, IPAddress* ipv4)
    {
        if (inet_aton(host, ipv4) == Status::Code::GOOD)
        {
            // host is already an IPv4 address
            return Status(Status::Code::GOOD);
        }
        
        uint8_t actualLength = 0;
        uint8_t buffer[128] = { 0 };
        createRequest(host, sizeof(buffer), &actualLength, buffer);

        uint8_t trialCount = 0;
        Status ret(Status::Code::UNCERTAIN);

        for (; trialCount < MAX_TRIAL_COUNT; ++trialCount)
        {
            ret = mSocket.SendTo(mDNSServer, DNS_PORT, actualLength, buffer);
            if (ret != Status::Code::GOOD)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (trialCount == MAX_TRIAL_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO SEND DNS REQUEST: %s", ret.c_str());
            return ret;
        }
        
        ret = waitForResponse();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "NO DNS RESPONSE: %s", ret.c_str());
            return ret;
        }

        uint16_t udpLengthField = 0;
        ret = verifyHeaderUDP(&udpLengthField);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID UDP HEADER: %s", ret.c_str());
            return ret;
        }

        ret = verifyHeaderDNS();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID DNS HEADER: %s", ret.c_str());
            return ret;
        }
        
        const size_t payloadLength = udpLengthField - DNS_HEADER_SIZE;
        ret = processPayload(host, payloadLength, ipv4);
        return ret;
    }
    
    
    Status DNS::inet_aton(const char* inputIPv4, IPAddress* outputIPv4)
    {
        ASSERT((inputIPv4  != nullptr), "INPUT PARAMETER CANNOT BE NULL");
        ASSERT((outputIPv4 != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        if (outputIPv4->fromString(inputIPv4) == false)
        {
            LOG_ERROR(logger, "INVALID IPv4 ADDRESS: %s", inputIPv4);
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    
    void DNS::createRequest(const char* host, const size_t length, uint8_t* actualLength, uint8_t* output)
    {
        ASSERT((host != nullptr), "HOST NAME CANNOT BE NULL");
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");
        ASSERT((length > (strlen(host) + 18)), "OUTPUT LENGTH MUST BE GREATER THAN %u(host + min length) Bytes", (strlen(host) + 18));

        memset(output, 0, length);
        uint8_t idx = 0;
        
        ++mRequestId;
        output[idx++] = mRequestId & 0xFF;
        output[idx++] = mRequestId >> 8;

        const uint16_t flags = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY | RECURSION_DESIRED_FLAG);
        output[idx++] = flags & 0xFF;
        output[idx++] = flags >> 8;

        const uint16_t qdCount = htons(1);
        output[idx++] = qdCount & 0xFF;
        output[idx++] = qdCount >> 8;
        idx = 0x0C;

        const char* start   = host;
        const char* finish  = start;
        uint8_t size = 0;

        while (*finish != '\0')
        {
            finish = start;
            while ((*finish != '\0') && (*finish != '.'))
            {
                ++finish;
            }
            
            if (finish - start > 0)
            {
                size = finish - start;

                memcpy(&output[idx], &size, sizeof(size));
                idx += sizeof(size);

                memcpy(&output[idx], start, size);
                idx += size;
            }

            start = finish + 1;
        }
        output[idx++] = 0x00; // terminate the query name with '\0'
        
        const uint16_t queryType = htons(TYPE_A);
        output[idx++] = queryType & 0xFF;
        output[idx++] = queryType >> 8;
        
        const uint16_t queryClass = htons(CLASS_IN);
        output[idx++] = queryClass & 0xFF; // lower byte of queryClass
        output[idx++] = queryClass >> 8; // upper byte of queryClass

        *actualLength = idx;
    }
    

    Status DNS::waitForResponse()
    {
        sir_t interrupt;
        Status ret(Status::Code::UNCERTAIN);
        const uint32_t startMillis = millis();

        while ((millis() - startMillis) < mTimeoutMillis)
        {
            ret = mSocket.GetInterrupt(&interrupt);
            if (ret != Status::Code::GOOD)
            {
                continue;
            }
            
            if (interrupt.RECEIVED == true)
            {
                return Status(Status::Code::GOOD);
                goto ON_RECEIVE;
            }
        }
        
        ret = Status::Code::BAD_TIMEOUT;

    ON_RECEIVE:
        return ret;
    }


    Status DNS::verifyHeaderUDP(uint16_t* udpPayloadLength)
    {
        ASSERT((udpPayloadLength != nullptr), "UDP PAYLOAD LENGTH CANNOT BE NULL");
        
        size_t actualLength = 0;
        uint8_t udpHeader[UDP_HEADER_SIZE] = { 0 };
        
        Status ret = mSocket.ReceiveFrom(mDNSServer, DNS_PORT, UDP_HEADER_SIZE, &actualLength, udpHeader);
        if ((ret != Status::Code::GOOD) || (actualLength != UDP_HEADER_SIZE))
        {
            LOG_ERROR(logger, "FAILED TO RECEIVE UDP HEADER");
            ret = Status::Code::BAD_DATA_LOST;
            return ret;
        }

        IPAddress remoteIP(udpHeader[0], udpHeader[1], udpHeader[2], udpHeader[3]);
        const uint16_t remotePort = (static_cast<uint16_t>(udpHeader[4]) << 8) | udpHeader[5];
        if ((remoteIP != mDNSServer) || (remotePort != DNS_PORT))
        {
            LOG_ERROR(logger, "INVALID DNS SERVER: %s:%u", remoteIP.toString().c_str(), remotePort);
            return Status(Status::Code::BAD_SERVER_URI_INVALID);
        }

        const uint16_t udpLengthField  = (static_cast<uint16_t>(udpHeader[6]) << 8) | udpHeader[7];
        const uint16_t remainedLength  = mSocket.Available();
        if (udpLengthField != remainedLength)
        {
            LOG_ERROR(logger, "RECEIVED PARTIALLY: %u != %u", udpLengthField, remainedLength);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        // LOG_DEBUG(logger, "Received %u bytes from DNS server, '%s:%u'", udpLengthField, remoteIP.toString().c_str(), remotePort);
        
        *udpPayloadLength = udpLengthField;
        return ret;
    }


    Status DNS::verifyHeaderDNS()
    {
        size_t actualLength = 0;
        uint8_t dnsHeader[DNS_HEADER_SIZE] = { 0 };

        Status ret = mSocket.ReceiveFrom(mDNSServer, DNS_PORT, DNS_HEADER_SIZE, &actualLength, dnsHeader);
        if ((ret != Status::Code::GOOD) || (actualLength != DNS_HEADER_SIZE))
        {
            LOG_ERROR(logger, "FAILED TO RECEIVE DNS RESPONSE");
            ret = Status::Code::BAD_DATA_LOST;
            return ret;
        }
        
        const uint16_t receivedID    = htons((static_cast<uint16_t>(dnsHeader[0]) << 8) | dnsHeader[1]);
        const uint16_t receivedFlag  = (static_cast<uint16_t>(dnsHeader[2]) << 8) | dnsHeader[3];
        // const uint16_t queryCount    = htons((static_cast<uint16_t>(dnsHeader[5]) << 8) | dnsHeader[4]);
        const uint16_t answerCount   = htons((static_cast<uint16_t>(dnsHeader[7]) << 8) | dnsHeader[6]);

        if (receivedID != mRequestId)
        {
            LOG_ERROR(logger, "INVALID DNS REQUEST ID: '%u' != '%u'", mRequestId, receivedID);
            ret = Status::Code::BAD_UNKNOWN_RESPONSE;
            goto ON_EXIT;
        }

        if ((receivedFlag & QUERY_RESPONSE_MASK) != static_cast<uint16_t>(RESPONSE_FLAG) ||
            (receivedFlag & TRUNCATION_FLAG) || (receivedFlag & RESP_MASK))
        {
            LOG_ERROR(logger, "INVALID DNS RESPONSE HEADER: 0x%02X", receivedFlag);
            ret = Status::Code::BAD_UNKNOWN_RESPONSE;
            goto ON_EXIT;
        }

        if (answerCount == 0)
        {
            LOG_ERROR(logger, "NO DNS RESPONSE RECEIVED");
            ret = Status::Code::BAD_NO_DATA;
            goto ON_EXIT;
        }

    ON_EXIT:
        return ret;
    }


    size_t DNS::parseQuerySection(const size_t length, const uint8_t payload[], std::string* qname, uint16_t* qtype, uint16_t* qclass)
    {
        ASSERT((qname  != nullptr), "QName CANNOT BE NULL");
        ASSERT((qtype  != nullptr), "QType CANNOT BE NULL");
        ASSERT((qclass != nullptr), "QClass CANNOT BE NULL");

        // LOG_DEBUG(logger, "length: %u", length);
        
        size_t idx = 0;
        while ((payload[idx] != 0) && (idx < length))
        {
            const uint8_t lableLength = payload[idx++];
            const uint8_t lableEnd    = idx + lableLength;
            for (; idx < lableEnd; ++idx)
            {
                *qname += static_cast<char>(payload[idx]);
            }
            *qname += '.';
        }
        Serial.println();
        ++idx;
        qname->pop_back();
        // LOG_DEBUG(logger, "QName: '%s'", qname->c_str());

        // QType (2 bytes)
        *qtype = (payload[idx] << 8) | payload[idx + 1];
        idx += 2;

        // QClass (2 bytes)
        *qclass = (payload[idx] << 8) | payload[idx + 1];
        idx += 2;

        return idx; // Query Section 끝난 위치 반환
    }


    Status DNS::processPayload(const char* host, const size_t payloadLength, IPAddress* resolvedIPv4)
    {
        ASSERT((host != nullptr), "HOST NAME CANNOT BE NULL");
        ASSERT((resolvedIPv4 != nullptr), "RESOLVED IPv4 CANNOT BE NULL");

        size_t actualLength = 0;
        uint8_t payload[payloadLength] = { 0 };
        Status ret = mSocket.ReceiveFrom(mDNSServer, DNS_PORT, payloadLength, &actualLength, payload);
        if ((ret != Status::Code::GOOD) || (actualLength != payloadLength))
        {
            LOG_ERROR(logger, "FAILED TO RECEIVE DNS PAYLOAD");
            ret = Status::Code::BAD_DATA_LOST;
            goto ON_ERROR;
        }
        else
        {
            std::string qname;
            uint16_t qtype   = 0;
            uint16_t qclass  = 0;
            
            size_t offset = parseQuerySection(actualLength, payload, &qname, &qtype, &qclass);
            if (strcmp(host, qname.c_str()) != 0)
            {
                LOG_ERROR(logger, "INVALID HOST NAME: '%s' != '%s'", host, qname.c_str());
                ret = Status::Code::BAD_UNKNOWN_RESPONSE;
                goto ON_ERROR;
            }
            
            if ((qtype != TYPE_A) && (qclass != CLASS_IN))
            {
                LOG_ERROR(logger, "INVALID QUERY TYPE OR CLASS: '%u', '%u'", qtype, qclass);
                ret = Status::Code::BAD_UNKNOWN_RESPONSE;
                goto ON_ERROR;
            }

            for (; offset < actualLength; ++offset)
            {
                if (payload[offset] != LABEL_COMPRESSION_MASK)
                {
                    continue;
                }
                else if (payload[offset] == LABEL_COMPRESSION_MASK)
                {
                    offset += 2;
                }
                
                uint16_t qtype   = (static_cast<uint16_t>(payload[offset]) << 8) | payload[offset + 1];
                offset += 2;

                uint16_t qclass  = (static_cast<uint16_t>(payload[offset]) << 8) | payload[offset + 1];
                offset += 2;

                if ((qtype != TYPE_A) && (qclass != CLASS_IN))
                {
                    continue; // Skip unsupported answer types
                }
                
                const uint32_t ttl = (static_cast<uint32_t>(payload[offset + 0]) << 24) | 
                                     (static_cast<uint32_t>(payload[offset + 1]) << 16) | 
                                     (static_cast<uint32_t>(payload[offset + 2]) <<  8) | 
                                     payload[offset + 3];
                offset += 4;
    
                const uint16_t dataLength = (static_cast<uint16_t>(payload[offset]) << 8) | payload[offset + 1];
                offset += 2;
                
                if (dataLength != 4)
                {
                    continue; // Skip unsupported answer types
                }
                
                LOG_DEBUG(logger, "TTL: %u", ttl);
                LOG_DEBUG(logger, "Data Length: %u", dataLength);

                IPAddress ipv4(&payload[offset]);
                if (ipv4 != IPADDR_NONE)
                {
                    // LOG_DEBUG(logger, "Resolved IP Address: %s", ipv4.toString().c_str());
                    *resolvedIPv4 = ipv4;
                    return Status(Status::Code::GOOD);
                }
            }
        }
        
    ON_ERROR:
        LOG_ERROR(logger, "FAILED TO RESOLVE '%s':", host, ret.c_str());
        return ret;
    }
}}

#endif