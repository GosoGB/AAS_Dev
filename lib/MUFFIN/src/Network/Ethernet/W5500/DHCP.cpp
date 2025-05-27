/**
 * @file DHCP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-08
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025
 */



#if defined(MT11)

#include <string.h>

#include "Common/Logger/Logger.h"
#include "DHCP.h"
#include "Socket.h"

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

    Status DHCP::Init(W5500& w5500, const uint8_t socketID, uint8_t buffer[])
    {
        memset(&mDHCP, 0, sizeof(mDHCP));
        initMessage(w5500);
        resetTimeout();

        // w5500.SetIPv4();
        // w5500.SetGateway();
        
        mSocket  = socketID;
        mState = state_e::INIT;
        return Status(Status::Code::GOOD);
    }


    Status DHCP::initMessage(W5500& w5500)
    {
        mDHCP.op    = op_e::BOOTREQUEST;
        mDHCP.hops  = 0;
        mDHCP.secs  = 0;

        uint8_t mac[6] = { 0 };
        Status ret = w5500.GetMacAddress(mac);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO GET MAC ADDRESS");
            return ret;
        }

        const uint8_t seed = mac[3] ^ mac[4] ^ mac[5];
        srand(seed);

        mDHCP.xid    = esp_random();
        mDHCP.flags  = flag_e::BROADCASTING;
        // uint8_t* pXID     = (uint8_t*)(&mDHCP.xid);
        // *(pXID + 0)       = (uint8_t)((DHCP_XID & 0xFF000000) >> 24);
        // *(pXID + 1)       = (uint8_t)((DHCP_XID & 0x00FF0000) >> 16);
        // *(pXID + 2)       = (uint8_t)((DHCP_XID & 0x0000FF00) >>  8);
        // *(pXID + 3)       = (uint8_t)((DHCP_XID & 0x000000FF) >>  0);   

        // uint8_t* pFLAG    = (uint8_t*)(&mDHCP.flags);
        // *(pFLAG + 0)      = (uint8_t)((static_cast<uint16_t>(flag_e::BROADCASTING) & 0xFF00) >> 8);
        // *(pFLAG + 1)      = (uint8_t)((static_cast<uint16_t>(flag_e::BROADCASTING) & 0x00FF) >> 0);

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


/*
    Status DHCP::sendRequest(W5500& w5500)
    {
        if (mState == state_e::LEASED || mState == state_e::REREQUSET)
        {
            mDHCP.flags = flag_e::UNICASTING;
            memcpy(mDHCP.ciaddr, mAllocatedIPv4, sizeof(mDHCP.ciaddr));
            // ip[0] = mDHCP.siaddr[0];
            // ip[1] = mDHCP.siaddr[1];
            // ip[2] = mDHCP.siaddr[2];
            // ip[3] = mDHCP.siaddr[3];   	   	   	
        }
        else
        {
            memset(mDHCP.siaddr, 0xFF, sizeof(mDHCP.siaddr));
        }
        
        uint16_t k = 4;      // because MAGIC_COOKIE already made by makeDHCPMSG()
            
        // Option Request Param.
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpMessageType);
        mDHCP.options[k++] = 0x01;
        mDHCP.options[k++] = static_cast<uint8_t>(type_e::REQUEST);
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpClientIdentifier);
        mDHCP.options[k++] = 0x07;
        mDHCP.options[k++] = 0x01;
        mDHCP.options[k++] = mDHCP.chaddr[0];
        mDHCP.options[k++] = mDHCP.chaddr[1];
        mDHCP.options[k++] = mDHCP.chaddr[2];
        mDHCP.options[k++] = mDHCP.chaddr[3];
        mDHCP.options[k++] = mDHCP.chaddr[4];
        mDHCP.options[k++] = mDHCP.chaddr[5];

        if (mDHCP.siaddr[3] == 255)
        {
            mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpRequestedIPaddr);
            mDHCP.options[k++] = 0x04;
            mDHCP.options[k++] = mDHCP.yiaddr[0];
            mDHCP.options[k++] = mDHCP.yiaddr[1];
            mDHCP.options[k++] = mDHCP.yiaddr[2];
            mDHCP.options[k++] = mDHCP.yiaddr[3];
        
            mDHCP.options[k++] = static_cast<uint8_t>(opt_e::dhcpServerIdentifier);
            mDHCP.options[k++] = 0x04;
            mDHCP.options[k++] = mDHCP.siaddr[0];
            mDHCP.options[k++] = mDHCP.siaddr[1];
            mDHCP.options[k++] = mDHCP.siaddr[2];
            mDHCP.options[k++] = mDHCP.siaddr[3];
        }

        // host name
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::hostName);
        mDHCP.options[k++] = 0; // length of hostname
        uint16_t i = 0;
        for (i = 0 ; HOST_NAME[i] != 0; i++)
        {
            mDHCP.options[k++] = HOST_NAME[i];
        }

        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[3] >> 4); 
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[3]);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[4] >> 4); 
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[4]);
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[5] >> 4); 
        mDHCP.options[k++] = NibbleToHex(mDHCP.chaddr[5]);
        mDHCP.options[k - (i+6+1)] = i+6; // length of hostname
        
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
        mDHCP.options[k++] = static_cast<uint8_t>(opt_e::END_OPTION);

        for (i = k; i < MAX_SIZE_OPT; i++)
        {
            mDHCP.options[i] = 0;
        }
        
        
        ;
        sendto(DHCP_SOCKET, (uint8_t*)mDHCP, MAX_SIZE_MSG, ip, SERVER_PORT);
    }
*/

    void DHCP::resetTimeout()
    {
        tickCurrent = 0;
        tickNext = WAIT_TIME;
        trialCount = 0;
    }


    void DHCP::sendDiscoverMessage(W5500& w5500)
    {
        uint16_t i;
        uint16_t k = 0;
    
        // makeDHCPMSG();
        memset(mServerIP, 0, sizeof(mServerIP));
        // DHCP_REAL_SIP[0]=0;
        // DHCP_REAL_SIP[1]=0;
        // DHCP_REAL_SIP[2]=0;
        // DHCP_REAL_SIP[3]=0;

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
        w5500.GetMacAddress(mac);
        mDHCP.options[k++] = mac[0];
        mDHCP.options[k++] = mac[1];
        mDHCP.options[k++] = mac[2];
        mDHCP.options[k++] = mac[3];
        mDHCP.options[k++] = mac[4];
        mDHCP.options[k++] = mac[5];
        // 0x44;
        // host name
        // mDHCP.options[k++] = static_cast<uint8_t>(opt_e::hostName);
        // mDHCP.options[k++] = 0;          // fill zero length of hostname 
        // for(i = 0 ; HOST_NAME[i] != 0; i++)
        // {
        //     mDHCP.options[k++] = HOST_NAME[i];
        // }
            
        // mDHCP.options[k++] = NibbleToHex(mac[3] >> 4); 
        // mDHCP.options[k++] = NibbleToHex(mac[3]);
        // mDHCP.options[k++] = NibbleToHex(mac[4] >> 4); 
        // mDHCP.options[k++] = NibbleToHex(mac[4]);
        // mDHCP.options[k++] = NibbleToHex(mac[5] >> 4); 
        // mDHCP.options[k++] = NibbleToHex(mac[5]);
        // mDHCP.options[k - (i+6+1)] = i+6; // length of hostname

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
    


        for (size_t i = 0; i < 258; i++)
        {
            Serial.printf("%u, ", buff[i]);
        }
        Serial.println();
        
        IPAddress hostIP(255,255,255,255);
        // SendTo(w5500, sock_id_e::SOCKET_1, hostIP, 67, buff, sizeof(buff));
        std::abort();
    }


/*
    Status DHCP::Run(const sock_id_e idx, W5500& w5500)
    {
        uint8_t  type;
        uint8_t  ret;
    
        if (mState == state_e::STOP)
        {
            LOG_ERROR(logger, "DHCP STATE MUST NOT BE STOP");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        // const uint8_t controlRead = Converter::ControlPhase(idx, am_e::READ);
        // uint8_t byteRead = 0x00;
        // Status ret = w5500.Read(srb_addr_e::MODE, controlRead, &byteRead);

        sock_prtcl_e protocol = w5500.GetSocketProtocol(idx);
        if (protocol != sock_prtcl_e::UDP)
        {
            smr_t socketMode;
            socketMode.PROTOCOL = sock_prtcl_e::UDP;
            Status ret = Socket(idx, socketMode, w5500);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO OPEN SOCKET FOR DHCP");
                return ret;
            }
        }
        
        // socket(DHCP_SOCKET, Sn_MR_UDP, CLIENT_PORT, 0x00);
        // ret = DHCP_RUNNING;
        
        type = parseDHCPMSG();
    
        switch (mState)
        {
           case state_e::INIT:
             mAllocatedIPv4[0] = 0;
             mAllocatedIPv4[1] = 0;
             mAllocatedIPv4[2] = 0;
             mAllocatedIPv4[3] = 0;
               send_DHCP_DISCOVER();
               mState = STATE_DHCP_DISCOVER;
               break;
            case STATE_DHCP_DISCOVER :
                if (type == DHCP_OFFER){
    #ifdef _DHCP_DEBUG_
                    printf("> Receive DHCP_OFFER\r\n");
    #endif
                mAllocatedIPv4[0] = mDHCP.yiaddr[0];
                mAllocatedIPv4[1] = mDHCP.yiaddr[1];
                mAllocatedIPv4[2] = mDHCP.yiaddr[2];
                mAllocatedIPv4[3] = mDHCP.yiaddr[3];
    
                    send_DHCP_REQUEST();
                    mState = STATE_DHCP_REQUEST;
                } else ret = check_DHCP_timeout();
             break;
    
            case STATE_DHCP_REQUEST :
                if (type == DHCP_ACK) {
    
    #ifdef _DHCP_DEBUG_
                    printf("> Receive DHCP_ACK\r\n");
    #endif
                    if (check_DHCP_leasedIP()) {
                        // Network info assignment from DHCP
                        dhcp_ip_assign();
                        reset_DHCP_timeout();
    
                        mState = state_e::LEASED;
                    } else {
                        // IP address conflict occurred
                        reset_DHCP_timeout();
                        dhcp_ip_conflict();
                        mState = STATE_DHCP_INIT;
                    }
                } else if (type == DHCP_NAK) {
    
    #ifdef _DHCP_DEBUG_
                    printf("> Receive DHCP_NACK\r\n");
    #endif
    
                    reset_DHCP_timeout();
    
                    mState = STATE_DHCP_DISCOVER;
                } else ret = check_DHCP_timeout();
            break;
    
            case state_e::LEASED :
               ret = DHCP_IP_LEASED;
                if ((mLeaseTime != INFINITE_LEASE_TIME) && ((mLeaseTime/2) < tickCurrent)) {
                    
    #ifdef _DHCP_DEBUG_
                     printf("> Maintains the IP address \r\n");
    #endif
    
                    type = 0;
                    mPreviousIPv4[0] = mAllocatedIPv4[0];
                    mPreviousIPv4[1] = mAllocatedIPv4[1];
                    mPreviousIPv4[2] = mAllocatedIPv4[2];
                    mPreviousIPv4[3] = mAllocatedIPv4[3];
                    
                    DHCP_XID++;
    
                    send_DHCP_REQUEST();
    
                    reset_DHCP_timeout();
    
                    mState = state_e::REREQUSET;
                }
            break;
    
            case state_e::REREQUSET :
               ret = DHCP_IP_LEASED;
                if (type == DHCP_ACK) {
                    trialCount = 0;
                    if (mPreviousIPv4[0] != mAllocatedIPv4[0] || 
                        mPreviousIPv4[1] != mAllocatedIPv4[1] ||
                        mPreviousIPv4[2] != mAllocatedIPv4[2] ||
                        mPreviousIPv4[3] != mAllocatedIPv4[3]) 
                    {
                        ret = DHCP_IP_CHANGED;
                        dhcp_ip_update();
                   #ifdef _DHCP_DEBUG_
                      printf(">IP changed.\r\n");
                   #endif
                        
                    }
             #ifdef _DHCP_DEBUG_
                else printf(">IP is continued.\r\n");
             #endif            				
                    reset_DHCP_timeout();
                    mState = state_e::LEASED;
                } else if (type == DHCP_NAK) {
    
    #ifdef _DHCP_DEBUG_
                    printf("> Receive DHCP_NACK, Failed to maintain ip\r\n");
    #endif
    
                    reset_DHCP_timeout();
    
                    mState = STATE_DHCP_DISCOVER;
                } else ret = check_DHCP_timeout();
               break;
            default :
               break;
        }
    
        return ret;
    }
*/  
    
    

    uint32_t DHCP::GetLeasetime() const
    {
        return mLeaseTime;
    }
}}



// #include "socket.h"
// #include "dhcp.h"

// #define DHCP_HOPS                0        ///< Used in hops of @ref rip_t
// #define DHCP_SECS                0        ///< Used in secs of @ref rip_t





uint8_t DHCP_REAL_SIP[4];                 // For extract my DHCP server in a few DHCP server

/* The default callback function */
void default_ip_assign(void);
void default_ip_update(void);
void default_ip_conflict(void);

/* Callback handler */
void (*dhcp_ip_assign)(void)   = default_ip_assign;     /* handler to be called when the IP address from DHCP server is first assigned */
void (*dhcp_ip_update)(void)   = default_ip_update;     /* handler to be called when the IP address from DHCP server is updated */
void (*dhcp_ip_conflict)(void) = default_ip_conflict;   /* handler to be called when the IP address from DHCP server is conflict */

void reg_dhcp_cbfunc(void(*ip_assign)(void), void(*ip_update)(void), void(*ip_conflict)(void));

char NibbleToHex(uint8_t nibble);
    
/* send DISCOVER message to DHCP server */
void     send_DHCP_DISCOVER(void);

/* send REQEUST message to DHCP server */
void     send_DHCP_REQUEST(void);

/* send DECLINE message to DHCP server */
void     send_DHCP_DECLINE(void);

/* IP conflict check by sending ARP-request to leased IP and wait ARP-response. */
int8_t   check_DHCP_leasedIP(void);

/* check the timeout in DHCP process */
uint8_t  check_DHCP_timeout(void);


/* Parse message as OFFER and ACK and NACK from DHCP server.*/
int8_t   parseDHCPCMSG(void);

/* The default handler of ip assign first */
void default_ip_assign(void)
{
//    setSIPR(mAllocatedIPv4);
//    setSUBR(mAllocatedSNM);
//    setGAR (mAllocatedGW);
}

/* The default handler of ip changed */
void default_ip_update(void)
{
// 	setMR(MR_RST);
// 	getMR(); // for delay

//    default_ip_assign();
//    setSHAR(DHCP_CHADDR);
}

/* The default handler of ip changed */
void default_ip_conflict(void)
{
	// setMR(MR_RST);
	// getMR(); // for delay
    
	// setSHAR(DHCP_CHADDR);
}

/* register the call back func. */
void reg_dhcp_cbfunc(void(*ip_assign)(void), void(*ip_update)(void), void(*ip_conflict)(void))
{
    dhcp_ip_assign   = default_ip_assign;
    dhcp_ip_update   = default_ip_update;
    dhcp_ip_conflict = default_ip_conflict;

    if (ip_assign)
    {
        dhcp_ip_assign = ip_assign;
    }

    if (ip_update)
    {
        dhcp_ip_update = ip_update;
    }
    
    if (ip_conflict)
    {
        dhcp_ip_conflict = ip_conflict;
    }
}

/*
// SEND DHCP DHCPDECLINE
void send_DHCP_DECLINE(void)
{
	int i;
	uint8_t ip[4];
	uint16_t k = 0;
	
	makeDHCPMSG();

   k = 4;      // because MAGIC_COOKIE already made by makeDHCPMSG()
   
	*((uint8_t*)(&mDHCP.flags))   = ((DHCP_FLAGSUNICAST & 0xFF00)>> 8);
	*((uint8_t*)(&mDHCP.flags)+1) = (DHCP_FLAGSUNICAST & 0x00FF);

	// Option Request Param.
	mDHCP.options[k++] = dhcpMessageType;
	mDHCP.options[k++] = 0x01;
	mDHCP.options[k++] = DHCP_DECLINE;

	mDHCP.options[k++] = dhcpClientIdentifier;
	mDHCP.options[k++] = 0x07;
	mDHCP.options[k++] = 0x01;
	mDHCP.options[k++] = DHCP_CHADDR[0];
	mDHCP.options[k++] = DHCP_CHADDR[1];
	mDHCP.options[k++] = DHCP_CHADDR[2];
	mDHCP.options[k++] = DHCP_CHADDR[3];
	mDHCP.options[k++] = DHCP_CHADDR[4];
	mDHCP.options[k++] = DHCP_CHADDR[5];

	mDHCP.options[k++] = dhcpRequestedIPaddr;
	mDHCP.options[k++] = 0x04;
	mDHCP.options[k++] = mAllocatedIPv4[0];
	mDHCP.options[k++] = mAllocatedIPv4[1];
	mDHCP.options[k++] = mAllocatedIPv4[2];
	mDHCP.options[k++] = mAllocatedIPv4[3];

	mDHCP.options[k++] = dhcpServerIdentifier;
	mDHCP.options[k++] = 0x04;
	mDHCP.options[k++] = DHCP_SIP[0];
	mDHCP.options[k++] = DHCP_SIP[1];
	mDHCP.options[k++] = DHCP_SIP[2];
	mDHCP.options[k++] = DHCP_SIP[3];

	mDHCP.options[k++] = endOption;

	for (i = k; i < MAX_SIZE_OPT; i++) mDHCP.options[i] = 0;

	//send broadcasting packet
	ip[0] = 0xFF;
	ip[1] = 0xFF;
	ip[2] = 0xFF;
	ip[3] = 0xFF;

	sendto(DHCP_SOCKET, (uint8_t *)mDHCP, MAX_SIZE_MSG, ip, SERVER_PORT);
}


// PARSE REPLY mDHCP
int8_t parseDHCPMSG(void)
{
	uint8_t svr_addr[6];
	uint16_t svr_port;
	uint16_t len;

	uint8_t* p;
	uint8_t* e;
	uint8_t type = 0;
	uint8_t opt_len;

#if 1
	// 20231019 taylor
	uint8_t addr_len;
#endif
   
   if ((len = getSn_RX_RSR(DHCP_SOCKET)) > 0)
   {
#if 1
	   // 20231019 taylor//teddy 240122
#if ((_WIZCHIP_ == 6100) || (_WIZCHIP_ == 6300))
	   len = recvfrom(DHCP_SOCKET, (uint8_t *)mDHCP, len, svr_addr, &svr_port, &addr_len);
#else
	   len = recvfrom(DHCP_SOCKET, (uint8_t *)mDHCP, len, svr_addr, &svr_port);
#endif
#else
	   len = recvfrom(DHCP_SOCKET, (uint8_t *)mDHCP, len, svr_addr, &svr_port);
#endif
   #ifdef _DHCP_DEBUG_   
      printf("DHCP message : %d.%d.%d.%d(%d) %d received. \r\n",svr_addr[0],svr_addr[1],svr_addr[2], svr_addr[3],svr_port, len);
   #endif   
   }
   else return 0;
	if (svr_port == SERVER_PORT) {
      // compare mac address
		if ( (mDHCP.chaddr[0] != DHCP_CHADDR[0]) || (mDHCP.chaddr[1] != DHCP_CHADDR[1]) ||
		     (mDHCP.chaddr[2] != DHCP_CHADDR[2]) || (mDHCP.chaddr[3] != DHCP_CHADDR[3]) ||
		     (mDHCP.chaddr[4] != DHCP_CHADDR[4]) || (mDHCP.chaddr[5] != DHCP_CHADDR[5])   )
		{
#ifdef _DHCP_DEBUG_
            printf("No My DHCP Message. This message is ignored.\r\n");
#endif
         return 0;
		}
        //compare DHCP server ip address
        if ((DHCP_SIP[0]!=0) || (DHCP_SIP[1]!=0) || (DHCP_SIP[2]!=0) || (DHCP_SIP[3]!=0)){
            if ( ((svr_addr[0]!=DHCP_SIP[0])|| (svr_addr[1]!=DHCP_SIP[1])|| (svr_addr[2]!=DHCP_SIP[2])|| (svr_addr[3]!=DHCP_SIP[3])) &&
                ((svr_addr[0]!=DHCP_REAL_SIP[0])|| (svr_addr[1]!=DHCP_REAL_SIP[1])|| (svr_addr[2]!=DHCP_REAL_SIP[2])|| (svr_addr[3]!=DHCP_REAL_SIP[3]))  )
            {
#ifdef _DHCP_DEBUG_
                printf("Another DHCP sever send a response message. This is ignored.\r\n");
#endif
                return 0;
            }
        }
		p = (uint8_t *)(&mDHCP.op);
		p = p + 240;      // 240 = sizeof(rip_t) + MAGIC_COOKIE size in rip_t.opt - sizeof(rip_t.opt)
		e = p + (len - 240);

		while ( p < e ) {

			switch ( *p ) {

   			case endOption :
   			   p = e;   // for break while(p < e)
   				break;
            case padOption :
   				p++;
   				break;
   			case dhcpMessageType :
   				p++;
   				p++;
   				type = *p++;
   				break;
   			case subnetMask :
   				p++;
   				p++;
   				mAllocatedSNM[0] = *p++;
   				mAllocatedSNM[1] = *p++;
   				mAllocatedSNM[2] = *p++;
   				mAllocatedSNM[3] = *p++;
   				break;
   			case routersOnSubnet :
   				p++;
   				opt_len = *p++;       
   				mAllocatedGW[0] = *p++;
   				mAllocatedGW[1] = *p++;
   				mAllocatedGW[2] = *p++;
   				mAllocatedGW[3] = *p++;
   				p = p + (opt_len - 4);
   				break;
   			case dns :
   				p++;                  
   				opt_len = *p++;       
   				mAllocatedDNS[0] = *p++;
   				mAllocatedDNS[1] = *p++;
   				mAllocatedDNS[2] = *p++;
   				mAllocatedDNS[3] = *p++;
   				p = p + (opt_len - 4);
   				break;
   			case dhcpIPaddrLeaseTime :
   				p++;
   				opt_len = *p++;
   				mLeaseTime  = *p++;
   				mLeaseTime  = (mLeaseTime << 8) + *p++;
   				mLeaseTime  = (mLeaseTime << 8) + *p++;
   				mLeaseTime  = (mLeaseTime << 8) + *p++;
            #ifdef _DHCP_DEBUG_  
               mLeaseTime = 10;
 				#endif
   				break;
   			case dhcpServerIdentifier :
   				p++;
   				opt_len = *p++;
   				DHCP_SIP[0] = *p++;
   				DHCP_SIP[1] = *p++;
   				DHCP_SIP[2] = *p++;
   				DHCP_SIP[3] = *p++;
                DHCP_REAL_SIP[0]=svr_addr[0];
                DHCP_REAL_SIP[1]=svr_addr[1];
                DHCP_REAL_SIP[2]=svr_addr[2];
                DHCP_REAL_SIP[3]=svr_addr[3];
   				break;
   			default :
   				p++;
   				opt_len = *p++;
   				p += opt_len;
   				break;
			} // switch
		} // while
	} // if
	return	type;
}

void  DHCP_stop(void)
{
   close(DHCP_SOCKET);
   mState = STATE_DHCP_STOP;
}

uint8_t check_DHCP_timeout(void)
{
	uint8_t ret = DHCP_RUNNING;
	
	if (trialCount < MAX_DHCP_RETRY) {
		if (tickNext < tickCurrent) {

			switch ( mState ) {
				case STATE_DHCP_DISCOVER :
//					printf("<<timeout>> state : STATE_DHCP_DISCOVER\r\n");
					send_DHCP_DISCOVER();
				break;
		
				case STATE_DHCP_REQUEST :
//					printf("<<timeout>> state : STATE_DHCP_REQUEST\r\n");

					send_DHCP_REQUEST();
				break;

				case state_e::REREQUSET :
//					printf("<<timeout>> state : state_e::REREQUSET\r\n");
					
					send_DHCP_REQUEST();
				break;
		
				default :
				break;
			}

			tickCurrent = 0;
			tickNext = tickCurrent + WAIT_TIME;
			trialCount++;
		}
	} else { // timeout occurred

		switch(mState) {
			case STATE_DHCP_DISCOVER:
				mState = STATE_DHCP_INIT;
				ret = DHCP_FAILED;
				break;
			case STATE_DHCP_REQUEST:
			case state_e::REREQUSET:
				send_DHCP_DISCOVER();
				mState = STATE_DHCP_DISCOVER;
				break;
			default :
				break;
		}
		reset_DHCP_timeout();
	}
	return ret;
}

int8_t check_DHCP_leasedIP(void)
{
	uint8_t tmp;
	int32_t ret;

	//WIZchip RCR value changed for ARP Timeout count control
	tmp = getRCR();
	setRCR(0x03);

	// IP conflict detection : ARP request - ARP reply
	// Broadcasting ARP Request for check the IP conflict using UDP sendto() function
	ret = sendto(DHCP_SOCKET, (uint8_t *)"CHECK_IP_CONFLICT", 17, mAllocatedIPv4, 5000);

	// RCR value restore
	setRCR(tmp);

	if (ret == SOCKERR_TIMEOUT) 
    {
		// UDP send Timeout occurred : allocated IP address is unique, DHCP Success
#ifdef _DHCP_DEBUG_
		printf("\r\n> Check leased IP - OK\r\n");
#endif
		return 1;
	}
    else
    {
		// Received ARP reply or etc : IP address conflict occur, DHCP Failed
		send_DHCP_DECLINE();

		ret = tickCurrent;
		while((tickCurrent - ret) < 2) ;   // wait for 1s over; wait to complete to send DECLINE message;

		return 0;
	}
}	
*/

void DHCP_time_handler(void)
{
	// tickCurrent++;
}

void getIPfromDHCP(uint8_t* ip)
{
	// ip[0] = mAllocatedIPv4[0];
	// ip[1] = mAllocatedIPv4[1];
	// ip[2] = mAllocatedIPv4[2];	
	// ip[3] = mAllocatedIPv4[3];
}

void getGWfromDHCP(uint8_t* ip)
{
	// ip[0] =mAllocatedGW[0];
	// ip[1] =mAllocatedGW[1];
	// ip[2] =mAllocatedGW[2];
	// ip[3] =mAllocatedGW[3];			
}

void getSNfromDHCP(uint8_t* ip)
{
//    ip[0] = mAllocatedSNM[0];
//    ip[1] = mAllocatedSNM[1];
//    ip[2] = mAllocatedSNM[2];
//    ip[3] = mAllocatedSNM[3];         
}

void getDNSfromDHCP(uint8_t* ip)
{
//    ip[0] = mAllocatedDNS[0];
//    ip[1] = mAllocatedDNS[1];
//    ip[2] = mAllocatedDNS[2];
//    ip[3] = mAllocatedDNS[3];         
}

// uint32_t getDHCPLeasetime(void)
// {
// 	return mLeaseTime;
// }

#endif