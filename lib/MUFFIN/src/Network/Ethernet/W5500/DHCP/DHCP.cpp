#if defined(MT11)

/**
 * @file DHCP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-29
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */



#include <string.h>

#include "Common/Logger/Logger.h"
#include "DHCP.h"
#include "../Socket.h"
#include "../W5500.h"

uint8_t HOST_NAME[] = DCHP_HOST_NAME;  


namespace muffin { namespace w5500 {


    const uint32_t DHCP::MAGIC_COOKIE;


    char NibbleToHex(uint8_t nibble)
    {
      nibble &= 0x0F;
      if (nibble <= 9)
        return nibble + '0';
      else 
        return nibble + ('A'-0x0A);
    }

    Status DHCP::Init()
    {
        memset(&mDHCP, 0, sizeof(mDHCP));
        initMessage();
        resetTimeout();

        mState = state_e::INIT;

        
        Status ret = mSocket.Open(static_cast<uint16_t>(dhcp_port_e::CLIENT));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO OPEN SOCKET : %s",ret.c_str());
            return ret;
        }
        
        size_t failCount = 1;
        while (failCount < 6)
        {
            ret = sendDiscoverMessage();
            if (ret == Status::Code::GOOD)
            {
                break;
            }
            LOG_WARNING(logger,"FAIL TO SEND DISCOVER MESSAGE [#%u] : %s",failCount,ret.c_str());
            failCount++;
            delay(1000);
        }
        
        if (failCount == 6)
        {
            LOG_ERROR(logger,"FAIL TO SEND DISCOVER MESSAGE");
            return Status(Status::Code::BAD);
        }
        
        
        size_t actualLength = 0;
        size_t receiveLength = mSocket.Available();
        uint8_t receiveOfferData[receiveLength] = {0};
        LOG_INFO(logger,"avilable : %u\n", receiveLength);
        ret = mSocket.Receive(receiveLength, &actualLength, receiveOfferData);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO RECEIVE OFFER MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }

        ret = parseOfferMessage(receiveOfferData, actualLength);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO PARSE OFFER MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }

        failCount = 1;
        while (failCount < 6)
        {
            ret = sendRequest();
            if (ret == Status::Code::GOOD)
            {
                break;
            }
            LOG_WARNING(logger,"FAIL TO SEND REQUEST MESSAGE [#%u] : %s",failCount,ret.c_str());
            failCount++;
            delay(1000);
        }
        
        if (failCount == 5)
        {
            LOG_ERROR(logger,"FAIL TO SEND REQUEST MESSAGE");
            return Status(Status::Code::BAD);
        }

        actualLength = 0;
        receiveLength = mSocket.Available();
        LOG_WARNING(logger,"receiveLength : %u",receiveLength);
        uint8_t receiveAckData[receiveLength] = {0};
        ret = mSocket.Receive(receiveLength, &actualLength, receiveAckData);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO RECEIVE ACK MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }
        ret = parseAckMessage(receiveAckData, actualLength);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO RECIEVE ACK MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }

        mSocket.mW5500.setLocalIP(IPAddress(mAllocatedIPv4[0], mAllocatedIPv4[1], mAllocatedIPv4[2], mAllocatedIPv4[3]));
        mSocket.mW5500.setGateway(IPAddress(mAllocatedGW[0], mAllocatedGW[1], mAllocatedGW[2], mAllocatedGW[3]));
        mSocket.mW5500.setSubnetmask(IPAddress(mAllocatedSNM[0], mAllocatedSNM[1], mAllocatedSNM[2], mAllocatedSNM[3]));
        mSocket.Close();

        return Status(Status::Code::GOOD);
    }


    Status DHCP::initMessage()
    {
        mDHCP.op    = op_e::BOOTREQUEST;
        mDHCP.hops  = 0;
        mDHCP.secs  = 0;
        
        uint8_t mac[6] = { 0 };
        
        Status ret = mSocket.mW5500.GetMacAddress(mac);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO GET MAC ADDRESS");
            return ret;
        }

        const uint8_t seed = mac[3] ^ mac[4] ^ mac[5];
        srand(seed);

        mDHCP.xid    = esp_random();
        mDHCP.flags  = flag_e::BROADCASTING;

        memset(mDHCP.ciaddr, 0, sizeof(mDHCP.ciaddr));
        memset(mDHCP.yiaddr, 0, sizeof(mDHCP.yiaddr));
        memset(mDHCP.siaddr, 0, sizeof(mDHCP.siaddr));
        memset(mDHCP.giaddr, 0, sizeof(mDHCP.giaddr));
        memset(mDHCP.chaddr, 0, sizeof(mDHCP.chaddr));
        mDHCP.chaddr[0] = mac[0];
        mDHCP.chaddr[1] = mac[1];
        mDHCP.chaddr[2] = mac[2];
        mDHCP.chaddr[3] = mac[3];
        mDHCP.chaddr[4] = mac[4];
        mDHCP.chaddr[5] = mac[5];
        memset(mDHCP.sname,  0, sizeof(mDHCP.sname));
        memset(mDHCP.file,   0, sizeof(mDHCP.file));
        memset(mDHCP.options, 0, sizeof(MAGIC_COOKIE));

        mDHCP.options[0] = static_cast<uint8_t>((MAGIC_COOKIE & 0xFF000000) >> 24);
        mDHCP.options[1] = static_cast<uint8_t>((MAGIC_COOKIE & 0x00FF0000) >> 16);
        mDHCP.options[2] = static_cast<uint8_t>((MAGIC_COOKIE & 0x0000FF00) >>  8);
        mDHCP.options[3] = static_cast<uint8_t>((MAGIC_COOKIE & 0x000000FF) >>  0);

        return Status(Status::Code::GOOD);
    }


    void DHCP::resetTimeout()
    {
        mTickCurrent = 0;
        trialCount = 0;
    }


    Status DHCP::sendDiscoverMessage()
    {
        uint16_t i;
        uint16_t k = 0;
    
        memset(mServerIP, 0, sizeof(mServerIP));
     
        k = 4;     // because MAGIC_COOKIE already made by makeDHCPMSG()
    
        // Option Request Param
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpMessageType);
        mDHCP.options[k++] = 0x01;
        mDHCP.options[k++] = static_cast<uint8_t>(state_e::DISCOVER);
        
        // Client identifier
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpClientIdentifier);
        mDHCP.options[k++] = 0x07;
        mDHCP.options[k++] = 0x01;

        uint8_t mac[6] = { 0 };
        mSocket.mW5500.GetMacAddress(mac);
        mDHCP.options[k++] = mac[0];
        mDHCP.options[k++] = mac[1];
        mDHCP.options[k++] = mac[2];
        mDHCP.options[k++] = mac[3];
        mDHCP.options[k++] = mac[4];
        mDHCP.options[k++] = mac[5];
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpParamRequest);
        mDHCP.options[k++] = 0x03;	// length of request
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::ROUTER_OPTION);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::SUBNETMASK);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dns);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::END_OPTION);

        for (i = k; i < MAX_SIZE_OPT; i++)
        {
            mDHCP.options[i] = 0;
        }

        uint8_t buff[258] = { 0 };
        buff[0] = static_cast<uint8_t>(mDHCP.op);
        buff[1] = mDHCP.htype;
        buff[2] = mDHCP.hlen;
        buff[3] = mDHCP.hops;
        buff[4] = (uint8_t)((mDHCP.xid & 0xFF000000) >> 24);
        buff[5] = (uint8_t)((mDHCP.xid & 0x00FF0000) >> 16);
        buff[6] = (uint8_t)((mDHCP.xid & 0x0000FF00) >>  8);
        buff[7] = (uint8_t)((mDHCP.xid & 0x000000FF) >>  0);
        buff[8] = (uint8_t)((mDHCP.secs & 0xFF00) >> 8);
        buff[9] = (uint8_t)((mDHCP.secs & 0x00FF) >> 0);
        buff[10] = (uint8_t)(((uint16_t)mDHCP.flags & 0xFF00) >> 8);
        buff[11] = (uint8_t)(((uint16_t)mDHCP.flags & 0x00FF) >> 0);
        buff[12] = mDHCP.ciaddr[0];
        buff[13] = mDHCP.ciaddr[1];
        buff[14] = mDHCP.ciaddr[2];
        buff[15] = mDHCP.ciaddr[3];
        buff[16] = mDHCP.yiaddr[0];
        buff[17] = mDHCP.yiaddr[1];
        buff[18] = mDHCP.yiaddr[2];
        buff[19] = mDHCP.yiaddr[3];
        buff[20] = mDHCP.siaddr[0];
        buff[21] = mDHCP.siaddr[1];
        buff[22] = mDHCP.siaddr[2];
        buff[23] = mDHCP.siaddr[3];
        buff[24] = mDHCP.giaddr[0];
        buff[25] = mDHCP.giaddr[1];
        buff[26] = mDHCP.giaddr[2];
        buff[27] = mDHCP.giaddr[3];
        buff[28] = mDHCP.chaddr[0];
        buff[29] = mDHCP.chaddr[1];
        buff[30] = mDHCP.chaddr[2];
        buff[31] = mDHCP.chaddr[3];
        buff[32] = mDHCP.chaddr[4];
        buff[33] = mDHCP.chaddr[5];
        buff[34] = mDHCP.chaddr[6];
        buff[35] = mDHCP.chaddr[7];
        buff[36] = mDHCP.chaddr[8];
        buff[37] = mDHCP.chaddr[9];
        buff[38] = mDHCP.chaddr[10];
        buff[39] = mDHCP.chaddr[11];
        buff[40] = mDHCP.chaddr[12];
        buff[41] = mDHCP.chaddr[13];
        buff[42] = mDHCP.chaddr[14];
        buff[43] = mDHCP.chaddr[15];

        for (size_t i = 44; i < 44 + 64; i++)
        {
            buff[i] = mDHCP.sname[i-44];
        }
        
        for (size_t i = 108; i < 108 + 128; i++)
        {
            buff[i] = mDHCP.file[i-108];
        }
        
        for (size_t i = 236; i < 236 + 22; i++)
        {
            buff[i] = mDHCP.options[i-236];
        }

        IPAddress hostIP(255,255,255,255);
        uint16_t timeoutMillis = 2000;
        uint32_t currentMillis = millis();

        Status ret = mSocket.SendTo(hostIP,static_cast<uint16_t>(dhcp_port_e::SERVER),sizeof(buff),buff);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO SEND DISCOVER MSG");
            return ret;
        }
        
        while (mSocket.Available() == 0)
        {
            if (millis() - currentMillis > timeoutMillis)
            {
                LOG_ERROR(logger,"TIMEOUT ERROR: FAIL TO SEND DISCOVER");
                return Status(Status::Code::BAD_TIMEOUT);
            }
        }

        mState = state_e::DISCOVER;

        return Status(Status::Code::GOOD);
    }

    Status DHCP::sendRequest()
    {
        // 1. 상태에 따라 flags 및 ciaddr 설정
        if (mState == state_e::LEASED || mState == state_e::RENEWING)
        {
            mDHCP.flags = flag_e::UNICASTING;
            memcpy(mDHCP.ciaddr, mAllocatedIPv4, sizeof(mDHCP.ciaddr));
        }
        else
        {
            memset(mDHCP.siaddr, 0x00, sizeof(mDHCP.siaddr));  // 기본값은 0
        }

        // OFFER로 받은 IP를 yiaddr에 저장
        memcpy(mDHCP.yiaddr, mAllocatedIPv4, 4);
        memcpy(mDHCP.siaddr, mServerIP, 4);  // BOOTP 필드 siaddr에도 저장

        uint16_t k = 4; // 옵션 시작 (MAGIC_COOKIE 이후)

        // Option 53: DHCP Message Type = REQUEST
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpMessageType);
        mDHCP.options[k++] = 0x01;
        mDHCP.options[k++] = static_cast<uint8_t>(type_e::REQUEST);

        // Option 61: Client Identifier (MAC)
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpClientIdentifier);
        mDHCP.options[k++] = 0x07;
        mDHCP.options[k++] = 0x01;
        for (int i = 0; i < 6; i++)
            mDHCP.options[k++] = mDHCP.chaddr[i];


        // Option 50: Requested IP Address
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpRequestedIPaddr);
        mDHCP.options[k++] = 0x04;
        memcpy(&mDHCP.options[k], mAllocatedIPv4, 4); k += 4;

        
        // Option 54: Server Identifier
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpServerIdentifier);
        mDHCP.options[k++] = 0x04;
        memcpy(&mDHCP.options[k], mServerIP, 4); k += 4;
        
        // Option 12: Host Name
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::hostName);
        mDHCP.options[k++] = 0; // 임시 자리 (길이 자리)

        uint16_t hostname_start = k;
        for (uint16_t i = 0; HOST_NAME[i] != 0; i++)
            mDHCP.options[k++] = HOST_NAME[i];

        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[3] >> 4);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[3]);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[4] >> 4);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[4]);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[5] >> 4);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[5]);

        mDHCP.options[hostname_start - 1] = k - hostname_start;  // 실제 길이 설정

        // Option 55: Parameter Request List
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpParamRequest);
        mDHCP.options[k++] = 0x08;
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::SUBNETMASK);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::ROUTER_OPTION);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dns);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::domainName);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpT1value);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpT2value);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::performRouterDiscovery);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::staticRoute);

        // Option 255: END
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::END_OPTION);

        // 나머지 옵션 0으로 패딩
        for (uint16_t i = k; i < MAX_SIZE_OPT; i++)
            mDHCP.options[i] = 0;
        
        
        uint8_t buff[548] = { 0 };
        buff[0] = static_cast<uint8_t>(mDHCP.op);
        buff[1] = mDHCP.htype;
        buff[2] = mDHCP.hlen;
        buff[3] = mDHCP.hops;
        buff[4] = (uint8_t)((mDHCP.xid & 0xFF000000) >> 24);
        buff[5] = (uint8_t)((mDHCP.xid & 0x00FF0000) >> 16);
        buff[6] = (uint8_t)((mDHCP.xid & 0x0000FF00) >>  8);
        buff[7] = (uint8_t)((mDHCP.xid & 0x000000FF) >>  0);
        buff[8] = (uint8_t)((mDHCP.secs & 0xFF00) >> 8);
        buff[9] = (uint8_t)((mDHCP.secs & 0x00FF) >> 0);
        buff[10] = (uint8_t)(((uint16_t)mDHCP.flags & 0xFF00) >> 8);
        buff[11] = (uint8_t)(((uint16_t)mDHCP.flags & 0x00FF) >> 0);
        buff[12] = mDHCP.ciaddr[0];
        buff[13] = mDHCP.ciaddr[1];
        buff[14] = mDHCP.ciaddr[2];
        buff[15] = mDHCP.ciaddr[3];
        buff[16] = mDHCP.yiaddr[0];
        buff[17] = mDHCP.yiaddr[1];
        buff[18] = mDHCP.yiaddr[2];
        buff[19] = mDHCP.yiaddr[3];
        buff[20] = mDHCP.siaddr[0];
        buff[21] = mDHCP.siaddr[1];
        buff[22] = mDHCP.siaddr[2];
        buff[23] = mDHCP.siaddr[3];
        buff[24] = mDHCP.giaddr[0];
        buff[25] = mDHCP.giaddr[1];
        buff[26] = mDHCP.giaddr[2];
        buff[27] = mDHCP.giaddr[3];
        buff[28] = mDHCP.chaddr[0];
        buff[29] = mDHCP.chaddr[1];
        buff[30] = mDHCP.chaddr[2];
        buff[31] = mDHCP.chaddr[3];
        buff[32] = mDHCP.chaddr[4];
        buff[33] = mDHCP.chaddr[5];
        buff[34] = mDHCP.chaddr[6];
        buff[35] = mDHCP.chaddr[7];
        buff[36] = mDHCP.chaddr[8];
        buff[37] = mDHCP.chaddr[9];
        buff[38] = mDHCP.chaddr[10];
        buff[39] = mDHCP.chaddr[11];
        buff[40] = mDHCP.chaddr[12];
        buff[41] = mDHCP.chaddr[13];
        buff[42] = mDHCP.chaddr[14];
        buff[43] = mDHCP.chaddr[15];

        for (size_t i = 44; i < 44 + 64; i++)
        {
            buff[i] = mDHCP.sname[i-44];
        }
        
        for (size_t i = 108; i < 108 + 128; i++)
        {
            buff[i] = mDHCP.file[i-108];
        }
        
        for (size_t i = 236; i < 236 + k; i++)
        {
            buff[i] = mDHCP.options[i-236];
        }
       
        IPAddress hostIP(255,255,255,255);
        uint16_t timeoutMillis = 2000;
        uint32_t currentMillis = millis();

        Status ret = mSocket.SendTo(hostIP,static_cast<uint16_t>(dhcp_port_e::SERVER),sizeof(buff),buff);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO SEND DISCOVER MSG");
            return ret;
        }
        
        while (mSocket.Available() == 0)
        {
            if (millis() - currentMillis > timeoutMillis)
            {
                LOG_ERROR(logger,"TIMEOUT ERROR: FAIL TO SEND DISCOVER");
                return Status(Status::Code::BAD_TIMEOUT);
            }
        }

        return Status(Status::Code::GOOD);
    }
    
    Status DHCP::parseOfferMessage(const uint8_t* buffer, size_t length)
    {
        if (length < 300) 
        {
            LOG_ERROR(logger, "LAK OF PACKET LENGTH ERROR : %zu", length);
            return Status(Status::Code::BAD);
        }

        // UDP 헤더 8바이트를 넘기고 DHCP 메시지 시작
        const uint8_t* dhcp = buffer + 8;

        // yiaddr (할당 제안 IP 주소)
        memcpy(mAllocatedIPv4, dhcp + 16, 4);
        LOG_INFO(logger, "OFFER - yiaddr : %u.%u.%u.%u",
                mAllocatedIPv4[0], mAllocatedIPv4[1], mAllocatedIPv4[2], mAllocatedIPv4[3]);

        // 로컬 MAC 로그
        LOG_DEBUG(logger, "Local MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                mDHCP.chaddr[0], mDHCP.chaddr[1], mDHCP.chaddr[2],
                mDHCP.chaddr[3], mDHCP.chaddr[4], mDHCP.chaddr[5]);

        // 수신 패킷 내 chaddr 로그
        LOG_DEBUG(logger, "Raw chaddr in packet: %02x:%02x:%02x:%02x:%02x:%02x",
                dhcp[28], dhcp[29], dhcp[30], dhcp[31], dhcp[32], dhcp[33]);

        // MAC 주소 일치 검사
        if (memcmp(dhcp + 28, mDHCP.chaddr, HLEN) != 0)
        {
            LOG_ERROR(logger, "MAC ADDRESS DOES NOT MATCH");
            return Status(Status::Code::BAD);
        }

        // DHCP 옵션 시작 위치 (BOOTP 236바이트 이후)

        if (memcmp(dhcp + 236, "\x63\x82\x53\x63", 4) != 0) 
        {
            LOG_ERROR(logger, "MAGIC COOKIE ERROR");
            return Status(Status::Code::BAD);
        }

        const uint8_t* options = dhcp + 240;
        const uint8_t* end = buffer + length;

        uint8_t dhcpMessageType = 0;

        while (options < end && *options != static_cast<uint8_t>(opt_e::END_OPTION))
        {
            uint8_t option = *options++;

            if (option == static_cast<uint8_t>(opt_e::PAD_OPTION))
            {
                continue;
            }

            if (options >= end)
            {
                break;
            }

            uint8_t len = *options++;
            if (options + len > end)
            {
                break;
            }

            switch (static_cast<opt_e>(option))
            {
                case opt_e::dhcpMessageType:
                    if (len == 1)
                    {
                        dhcpMessageType = options[0];
                    }
                    break;

                case opt_e::SUBNETMASK:
                    if (len == 4) 
                    {
                        memcpy(mAllocatedSNM, options, 4);
                        LOG_INFO(logger, "Subnet Mask: %u.%u.%u.%u",
                                mAllocatedSNM[0], mAllocatedSNM[1], mAllocatedSNM[2], mAllocatedSNM[3]);
                    }
                    break;

                case opt_e::ROUTER_OPTION:
                    if (len >= 4) 
                    {
                        memcpy(mAllocatedGW, options, 4);
                        LOG_INFO(logger, "Gateway: %u.%u.%u.%u",
                                mAllocatedGW[0], mAllocatedGW[1], mAllocatedGW[2], mAllocatedGW[3]);
                    }
                    break;

                case opt_e::dns:
                    if (len >= 4) 
                    {
                        memcpy(mAllocatedDNS, options, 4);
                        LOG_INFO(logger, "DNS: %u.%u.%u.%u",
                                mAllocatedDNS[0], mAllocatedDNS[1], mAllocatedDNS[2], mAllocatedDNS[3]);
                    }
                    break;

                case opt_e::dhcpIPaddrLeaseTime:
                    if (len == 4)
                    {
                        mLeaseTime = (options[0] << 24) | (options[1] << 16) |
                                    (options[2] << 8)  | (options[3]);
                        LOG_INFO(logger, "Lease Time: %us", mLeaseTime);
                    }
                    break;

                case opt_e::dhcpServerIdentifier:
                    if (len == 4) 
                    {
                        memcpy(mServerIP, options, 4);
                        LOG_INFO(logger, "Server Identifier: %u.%u.%u.%u",
                                mServerIP[0], mServerIP[1], mServerIP[2], mServerIP[3]);
                    }
                    break;

                default:
                    break;
            }
            options += len;
        }

        // 메시지 타입 확인
        if (dhcpMessageType != static_cast<uint8_t>(type_e::OFFER))
        {
            return Status(Status::Code::BAD);
        }

        // 상태 업데이트
        mState = state_e::REQUEST;
        LOG_INFO(logger, "Success offer message parsing");


        return Status(Status::Code::GOOD);
    }

    Status DHCP::parseAckMessage(const uint8_t* buffer, size_t length)
    {
        if (length < 300) 
        {
            LOG_ERROR(logger, "LAK OF PACKET LENGTH ERROR : %zu", length);
        }

        // UDP 헤더 8바이트를 넘기고 DHCP 메시지 시작
        const uint8_t* dhcp = buffer + 8;

        memcpy(mAllocatedIPv4, dhcp + 16, 4);  // yiaddr

        LOG_INFO(logger, "ACK - IP : %u.%u.%u.%u",
                mAllocatedIPv4[0], mAllocatedIPv4[1],
                mAllocatedIPv4[2], mAllocatedIPv4[3]);

        // MAC 확인
        if (memcmp(dhcp + 28, mDHCP.chaddr, 6) != 0)
        {
            LOG_ERROR(logger, "MAC ADDRESS DOES NOT MATCH(ACK)");
            return Status(Status::Code::BAD);
        }

        // DHCP 옵션 시작: 고정 위치 240 (BOOTP 고정 길이 236 + 4 byte magic cookie)
        const uint8_t* options = dhcp + 240;
        const uint8_t* end = buffer + length;

        uint8_t dhcpMessageType = 0;

        while (options < end && *options != static_cast<uint8_t>(opt_e::END_OPTION))
        {
            uint8_t option = *options++;

            if (option == static_cast<uint8_t>(opt_e::PAD_OPTION))
            {
                continue;
            }

            if (options >= end) 
            {
                break;
            }

            uint8_t len = *options++;
            if (options + len > end)
            {
                break;
            }

            switch (static_cast<opt_e>(option))
            {
                case opt_e::dhcpMessageType:
                    if (len == 1)
                    {
                        dhcpMessageType = options[0];
                        LOG_DEBUG(logger, "MessageType: %u (ACK = 5)", dhcpMessageType);
                    }
                    break;

                case opt_e::SUBNETMASK:
                    if (len == 4)
                    {
                        memcpy(mAllocatedSNM, options, 4);
                        LOG_INFO(logger, "Subnet Mask: %u.%u.%u.%u",
                                mAllocatedSNM[0], mAllocatedSNM[1],
                                mAllocatedSNM[2], mAllocatedSNM[3]);
                    }
                    break;

                case opt_e::ROUTER_OPTION:
                    if (len >= 4)
                    {
                        memcpy(mAllocatedGW, options, 4);
                        LOG_INFO(logger, "Gateway: %u.%u.%u.%u",
                                mAllocatedGW[0], mAllocatedGW[1],
                                mAllocatedGW[2], mAllocatedGW[3]);
                    }
                    break;

                case opt_e::dns:
                    if (len >= 4)
                    {
                        memcpy(mAllocatedDNS, options, 4);
                        LOG_INFO(logger, "DNS: %u.%u.%u.%u",
                                mAllocatedDNS[0], mAllocatedDNS[1],
                                mAllocatedDNS[2], mAllocatedDNS[3]);
                    }
                    break;

                case opt_e::dhcpIPaddrLeaseTime:
                    if (len == 4)
                    {
                        mLeaseTime = (options[0] << 24) | (options[1] << 16) |
                                    (options[2] << 8)  | options[3];

                        LOG_INFO(logger, "Lease Time: %us", mLeaseTime);
                    }
                    break;

                case opt_e::dhcpServerIdentifier:
                    if (len == 4)
                    {
                        memcpy(mServerIP, options, 4);
                        LOG_INFO(logger, "Server Identifier: %u.%u.%u.%u",
                                mServerIP[0], mServerIP[1], mServerIP[2], mServerIP[3]);
                    }
                    break;

                default:
                    break;
            }

            options += len;
        }

        if (dhcpMessageType != static_cast<uint8_t>(type_e::ACK))
        {
            return Status(Status::Code::BAD);
        }

        mState = state_e::LEASED;
        LOG_INFO(logger, "Success ACK message parsing");

        mTickCurrent = 0;
        mLeaseT1 = (mLeaseTime / 2);                // T1
        mLeaseT2 = (mLeaseTime * (0.75));           // T2
        LOG_INFO(logger, "mLeaseTime :%d, mLeaseT1 :%d, mLeaseT2 :%d",mLeaseTime, mLeaseT1,mLeaseT2);

        return Status(Status::Code::GOOD);
    }

    uint32_t DHCP::GetLeasetime() const
    {
        return mLeaseTime;
    }

    Status DHCP::renewingMessage()
    {
        mSocket.Open(static_cast<uint16_t>(dhcp_port_e::CLIENT));

        Status ret = sendRequest();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO SEND REQUEST MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }

        size_t actualLength = 0;
        size_t receiveLength = mSocket.Available();
        uint8_t receiveAckData[receiveLength] = {0};
        mSocket.Receive(receiveLength, &actualLength, receiveAckData);
        ret = parseAckMessage(receiveAckData, actualLength);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger,"FAIL TO RECIEVE ACK MESSAGE : %s",ret.c_str());
            mSocket.Close();
            return ret;
        }

        mSocket.mW5500.setLocalIP(IPAddress(mAllocatedIPv4[0], mAllocatedIPv4[1], mAllocatedIPv4[2], mAllocatedIPv4[3]));
        mSocket.mW5500.setGateway(IPAddress(mAllocatedGW[0], mAllocatedGW[1], mAllocatedGW[2], mAllocatedGW[3]));
        mSocket.mW5500.setSubnetmask(IPAddress(mAllocatedSNM[0], mAllocatedSNM[1], mAllocatedSNM[2], mAllocatedSNM[3]));
        mSocket.Close();
        return Status(Status::Code::GOOD);
    }

    Status DHCP::Run()
    {
        if (xTaskDhcpHandle != NULL)
        {
            LOG_WARNING(logger, "THE DHCP TASK HAS ALREADY STARTED");
            return Status(Status::Code::GOOD);
        }

        xTaskDhcpHandle = xSemaphoreCreateMutex();
        if (xTaskDhcpHandle == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE DHCP SEMAPHORE");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }


        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            taskEntryPoint,         // Function to be run inside of the task
            "DhcpTask",             // The identifier of this task for men
            4 * 1024,               // Stack memory size to allocate
            this,                   // Task parameters to be passed to the function
            1,				        // Task Priority for scheduling
            &xTaskDhcpHandle,       // The identifier of this task for machines
            1				        // Index of MCU core where the function to run
        );

        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The dhcp task has been started");
            return Status(Status::Code::GOOD);

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

    }

    void DHCP::taskEntryPoint(void* param) 
    {
        DHCP* instance = static_cast<DHCP*>(param);
        instance->taskLoop();
    }

    void DHCP::taskLoop()
    {
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(60000));  // 60초마다
            
            mTickCurrent += 60;

            LOG_DEBUG(logger, "Tick: %d, T1: %d, T2: %d, LeaseTime: %d", mTickCurrent, mLeaseT1, mLeaseT2, mLeaseTime);

            if (mTickCurrent >= mLeaseTime)
            {
                mState = state_e::RELEASE;
            }
            else if (mTickCurrent >= mLeaseT2)
            {
                mState = state_e::REBIND;
            }
            else if (mTickCurrent >= mLeaseT1)
            {
                mState = state_e::RENEWING;
            }

            switch (mState)
            {
                case state_e::RENEWING:
                case state_e::REBIND:
                    renewingMessage();
                    break;
                case state_e::RELEASE:
                    Init();
                    break;
                default:
                    break;
            }
        }
    }
}}


#endif