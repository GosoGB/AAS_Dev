#if defined(MT11)

/**
 * @file DHCP.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-29
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#pragma once

#include <sys/_stdint.h>

#include "Common/Status.h"
#include "../Socket.h"



namespace muffin { namespace w5500 {


    constexpr uint8_t HLEN = 6; // hardware address length
    constexpr uint16_t MAX_SIZE_OPT = 312;
    constexpr uint16_t MAX_SIZE_MSG = 236 + MAX_SIZE_OPT;
    constexpr uint32_t INFINITE_LEASE_TIME = UINT32_MAX;

    
    /* 
    * @brief return value of @ref DHCP_run()
    */
    typedef enum class DhcpReturnValueEnum
        : uint8_t 
    {
        DHCP_FAILED       = 0,
        DHCP_RUNNING      = 1,
        DHCP_IP_ASSIGN    = 2,  //< First Occupy IP from DHPC server      (if cbfunc == null, act as default default_ip_assign)
        DHCP_IP_CHANGED   = 3,  //< Change IP address by new ip from DHCP (if cbfunc == null, act as default default_ip_update)
        DHCP_IP_LEASED    = 4,  //< Stand by 
        DHCP_STOPPED      = 5   //< Stop processing DHCP protocol
    } ret_e;


    typedef enum class DhcpStateEnum
        : uint8_t 
    {
        INIT       = 0,
        DISCOVER   = 1,  // Send discover and wait for offer message
        REQUEST    = 2,  // Send request and wait for ack or nack message
        LEASED     = 3,  // Received ack and IP has been leased
        RENEWING   = 4,  // Send request for maintaining leased IP
        REBIND     = 5,
        RELEASE    = 6,  // Leased IP address has been released and no longer in use
        STOP       = 7   // Stop using DHCP
    } state_e;


    typedef enum class DhcpOperationCodeEnum
        : uint8_t
    {
        BOOTREQUEST   = 0x01,
        BOOTREPLY     = 0x02
    } op_e;


    typedef enum class DhcpFlagEnum
        : uint16_t
    {
        UNICASTING    = 0x0000, // Default
        BROADCASTING  = 0x8000
    } flag_e;


    typedef enum class DhcpMessageTypeEnum
        : uint8_t
    {
        DISCOVER   = 0x01,
        OFFER      = 0x02,
        REQUEST    = 0x03,
        DECLINE    = 0x04,
        ACK        = 0x05,
        NAK        = 0x06,
        RELEASE    = 0x07,
        INFORM     = 0x08
    } type_e;


/* 
 * @brief DHCP option and value (cf. RFC 1533)
 */
typedef enum class DhcpOptionsEnum
    : uint8_t
{
   PAD_OPTION              = 0,
   SUBNETMASK              = 1,
   TIME_OFFSET             = 2,
   ROUTER_OPTION           = 3,
   TIME_SERVER             = 4,
   nameServer              = 5,
   dns                     = 6,
   logServer               = 7,
   cookieServer            = 8,
   lprServer               = 9,
   impressServer           = 10,
   resourceLocationServer	= 11,
   hostName                = 12,
   bootFileSize            = 13,
   meritDumpFile           = 14,
   domainName              = 15,
   swapServer              = 16,
   rootPath                = 17,
   extentionsPath          = 18,
   IPforwarding            = 19,
   nonLocalSourceRouting   = 20,
   policyFilter            = 21,
   maxDgramReasmSize       = 22,
   defaultIPTTL            = 23,
   pathMTUagingTimeout     = 24,
   pathMTUplateauTable     = 25,
   ifMTU                   = 26,
   allSubnetsLocal         = 27,
   broadcastAddr           = 28,
   performMaskDiscovery    = 29,
   maskSupplier            = 30,
   performRouterDiscovery  = 31,
   routerSolicitationAddr  = 32,
   staticRoute             = 33,
   trailerEncapsulation    = 34,
   arpCacheTimeout         = 35,
   ethernetEncapsulation   = 36,
   tcpDefaultTTL           = 37,
   tcpKeepaliveInterval    = 38,
   tcpKeepaliveGarbage     = 39,
   nisDomainName           = 40,
   nisServers              = 41,
   ntpServers              = 42,
   vendorSpecificInfo      = 43,
   netBIOSnameServer       = 44,
   netBIOSdgramDistServer	= 45,
   netBIOSnodeType         = 46,
   netBIOSscope            = 47,
   xFontServer             = 48,
   xDisplayManager         = 49,
   dhcpRequestedIPaddr     = 50,
   dhcpIPaddrLeaseTime     = 51,
   dhcpOptionOverload      = 52,
   dhcpMessageType         = 53,
   dhcpServerIdentifier    = 54,
   dhcpParamRequest        = 55,
   dhcpMsg                 = 56,
   dhcpMaxMsgSize          = 57,
   dhcpT1value             = 58,
   dhcpT2value             = 59,
   dhcpClassIdentifier     = 60,
   dhcpClientIdentifier    = 61,
   END_OPTION               = 255
} opt_e;


typedef struct DynamicHostConfigurationProtocolType
{
    op_e op; // Message type
    static const uint8_t htype  = 0x01;   // Default value for hardware address type
    static const uint8_t hlen   = 0x06;   // Default value for hardware address length
    uint8_t  hops;        // Client sets to zero used by gateways optionally
    uint32_t xid;         // Transaction ID, random number, used to match boot request
    uint16_t secs;        // Seconds elapsed since a client obtained or renewed an IP address
    flag_e   flags;       // Transmission method for a reply message
    uint8_t  ciaddr[4];   // IPv4 address of a client. This field can be filled only when the client is in the Bound, Renew, or Rebinding state and can respond to ARP requests.
    uint8_t  yiaddr[4];   // IPv4 address that a server assigns to a client.
    uint8_t  siaddr[4];   // IPv4 address of the server to be used in the next phase of the DHCP process. 
    uint8_t  giaddr[4];   // IPv4 address of the first DHCP relay agent.
    uint8_t  chaddr[16];  // MAC address of a client. This field must be consistent with the Htype and Hlen fields. 
    uint8_t  sname[64];   // Name of the server from which a client obtains the configuration. This field is optional and is filled in by a DHCP server.
    uint8_t  file[128];   // Boot file name specified by the DHCP server for a DHCP client. This field is optional.
    uint8_t  options[MAX_SIZE_OPT];  // DHCP options field.
} dhcp_t;

    
    // uint8_t DHCP_SOCKET;
    // uint32_t DHCP_XID;      // Any number



    
    class DHCP
    {
    public: 
        DHCP(Socket& socket) : mSocket(socket) {}
        ~DHCP() {}

    public:
        /*
         * @brief DHCP client initialization (outside of the main loop)
         * @param s   - socket number
         * @param buf - buffer for processing DHCP message
         */
        Status Init();
        
        /*
         * @brief DHCP 1s Tick Timer handler
         * @note SHOULD BE register to your system 1s Tick timer handler 
         */
        Status DHCP_time_handler(void);
        
        /* 
         * @brief Register call back function 
         * @param ip_assign   - callback func when IP is assigned from DHCP server first
         * @param ip_update   - callback func when IP is changed
         * @param ip_conflict - callback func when the assigned IP is conflict with others.
         */
        void reg_dhcp_cbfunc(void(*ip_assign)(void), void(*ip_update)(void), void(*ip_conflict)(void));
        
        /*
         * @brief DHCP client in the main loop
         * @return    The value is as the follow \n
         *            @ref DHCP_FAILED     \n
         *            @ref DHCP_RUNNING    \n
         *            @ref DHCP_IP_ASSIGN  \n
         *            @ref DHCP_IP_CHANGED \n
         * 			  @ref DHCP_IP_LEASED  \n
         *            @ref DHCP_STOPPED    \n
         *
         * @note This function is always called by you main task.
         */ 
        Status Run();
        
        /*
         * @brief Stop DHCP processing
         * @note If you want to restart. call DHCP_init() and DHCP_run()
         */ 
        void    DHCP_stop(void);
        
        /* Get Network information assigned from DHCP server */
        /*
         * @brief Get IP address
         * @param ip  - IP address to be returned
         */
        void getIPfromDHCP(uint8_t* ip);
        /*
         * @brief Get Gateway address
         * @param ip  - Gateway address to be returned
         */
        void getGWfromDHCP(uint8_t* ip);
        /*
         * @brief Get Subnet mask value
         * @param ip  - Subnet mask to be returned
         */
        void getSNfromDHCP(uint8_t* ip);
        /*
         * @brief Get DNS address
         * @param ip  - DNS address to be returned
         */
        void getDNSfromDHCP(uint8_t* ip);
        
        /*
         * @brief Get the leased time by DHCP sever
         * @return unit 1s
         */
        uint32_t GetLeasetime() const;

    private:
        Status renewingMessage();
        Status initMessage();
        Status sendDiscoverMessage();
        Status sendRequest();
        Status parseAckMessage(const uint8_t* buffer, size_t length);
        Status parseOfferMessage(const uint8_t* buffer, size_t length);
        static void taskEntryPoint(void* param);
        void taskLoop();
 
    public:
        /* Reset the DHCP timeout count and retry count. */
        void resetTimeout();
    
    private:
        Socket& mSocket;
        state_e mState = state_e::INIT;
        uint32_t mLeaseTime      = INFINITE_LEASE_TIME;
        uint32_t mLeaseT2        = 0;
        uint32_t mLeaseT1        = 0;
        uint32_t mTickCurrent    = 0;
        uint8_t  trialCount      = 0;
        TaskHandle_t xTaskDhcpHandle = nullptr;

    private:
        // uint8_t mMAC[6] = { 0 };    // DHCP_CHADDR
        uint8_t mServerIP[4]        = { 0 };
        uint8_t mPreviousIPv4[4]    = { 0 };
        uint8_t mAllocatedIPv4[4]   = { 0 };
        uint8_t mAllocatedGW[4]     = { 0 };
        uint8_t mAllocatedSNM[4]    = { 0 };
        uint8_t mAllocatedDNS[4]    = { 0 };
        
        uint8_t mSocketID;
        uint32_t mXID = 0x12345678;
        dhcp_t mDHCP;
    private:
        static const uint8_t   MAX_TRIAL_COUNT  =  5;  // MAX_DHCP_RETRY
        static const uint8_t   WAIT_TIME        = 10;  // WAIT_TIME  // unit: second
        static const uint8_t   SERVER_PORT      = 67;
        static const uint8_t   CLIENT_PORT      = 68;
        static const uint32_t  MAGIC_COOKIE     = 0x63825363;
    };
}}
#define DCHP_HOST_NAME           "MT11-\0"


#endif