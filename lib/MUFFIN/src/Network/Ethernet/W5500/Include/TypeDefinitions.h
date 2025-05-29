/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Common Register Block 클래스를 선언합니다.
 * 
 * @version 1.0.0
 * @date 2025-04-22
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin { namespace w5500 {


    typedef enum class W5500InterfaceEnum
        : uint8_t
    {
        EMBEDDED   = 10,
        LINK_01    = 21,
        LINK_02    = 17
    } if_e;

    typedef enum class DhcpPortEnum 
        : uint16_t
    {
        SERVER = 67,  // DHCP 서버가 수신하는 포트
        CLIENT = 68   // DHCP 클라이언트가 수신하는 포트
    } dhcp_port_e;

    typedef enum class BlockSelectBitsEnum
        : uint8_t
    {
        COMMON_REGISTER       = 0x00,
        SOCKET_0_REGISTER     = 0x01,
        SOCKET_0_Tx_BUFFER    = 0x02,
        SOCKET_0_Rx_BUFFER    = 0x03,
        SOCKET_1_REGISTER     = 0x05,
        SOCKET_1_Tx_BUFFER    = 0x06,
        SOCKET_1_Rx_BUFFER    = 0x07,
        SOCKET_2_REGISTER     = 0x09,
        SOCKET_2_Tx_BUFFER    = 0x0A,
        SOCKET_2_Rx_BUFFER    = 0x0B,
        SOCKET_3_REGISTER     = 0x0D,
        SOCKET_3_Tx_BUFFER    = 0x0E,
        SOCKET_3_Rx_BUFFER    = 0x0F,
        SOCKET_4_REGISTER     = 0x11,
        SOCKET_4_Tx_BUFFER    = 0x12,
        SOCKET_4_Rx_BUFFER    = 0x13,
        SOCKET_5_REGISTER     = 0x15,
        SOCKET_5_Tx_BUFFER    = 0x16,
        SOCKET_5_Rx_BUFFER    = 0x17,
        SOCKET_6_REGISTER     = 0x19,
        SOCKET_6_Tx_BUFFER    = 0x1A,
        SOCKET_6_Rx_BUFFER    = 0x1B,
        SOCKET_7_REGISTER     = 0x1D,
        SOCKET_7_Tx_BUFFER    = 0x1E,
        SOCKET_7_Rx_BUFFER    = 0x1F
    } bsb_e;
    

    typedef enum class AccessModeEnum
        : uint8_t
    {
        READ   = 0x00,
        WRITE  = 0x01
    } am_e;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0000
     */
    typedef struct ModeRegisterType
    {
        bool RST    : 1;    // software reset flag
        bool WoL    : 1;    // Wake-on-LAN flag
        bool PB     : 1;    // Ping block mode flag
        bool PPPoE  : 1;    // PPPoE mode flag
        bool FARP   : 1;    // Force ARP mode flag
    } mr_t;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0015
     */
    typedef struct InterruptRegisterType
    {
        bool Conflict      : 1;  // Source IPv4 conflict by ARP request
        bool Unreachable   : 1;  // Unreachable destination -> read UIPR, UPORTR
        bool PPPoE         : 1;  // PPPoE connection is closed
        bool MagicPacket   : 1;  // Received magic packet for WoL
    } ir_t;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0015
     */
    typedef struct InterruptMaskRegisterType
    {
        bool Conflict      : 1;  // Source IPv4 conflict by ARP request
        bool Unreachable   : 1;  // Unreachable destination -> read UIPR, UPORTR
        bool PPPoE         : 1;  // PPPoE connection is closed
        bool MagicPacket   : 1;  // Received magic packet for WoL
    } imr_t;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0017
     */
    typedef struct CommonRegisterBlockSocketInterruptRegisterType
    {
        bool Socket_0   : 1;
        bool Socket_1   : 1;
        bool Socket_2   : 1;
        bool Socket_3   : 1;
        bool Socket_4   : 1;
        bool Socket_5   : 1;
        bool Socket_6   : 1;
        bool Socket_7   : 1;
    } crb_sir_t;


    typedef enum class SocketIndexEnum
        : uint8_t 
    {
        SOCKET_0  = 0,
        SOCKET_1  = 1,
        SOCKET_2  = 2,
        SOCKET_3  = 3,
        SOCKET_4  = 4,
        SOCKET_5  = 5,
        SOCKET_6  = 6,
        SOCKET_7  = 7,
        TOP       = 8
    } sock_id_e;



    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0018
     */
    typedef struct SocketInterruptMaskRegisterType
    {
        bool Socket_0   : 1;
        bool Socket_1   : 1;
        bool Socket_2   : 1;
        bool Socket_3   : 1;
        bool Socket_4   : 1;
        bool Socket_5   : 1;
        bool Socket_6   : 1;
        bool Socket_7   : 1;
    } simr_t;
    

    /**
     * @brief
     *   - Access Mode: Read for all, Write partially
     *   - Default Value: 0b10111xxx
     *   - Address: 0x002E
     */
    typedef struct PhyConfigurationType
    {
        bool Reset    : 1;  // Reset PHY when set to 0
        bool Mode     : 1;  // Configure using OPMDC when set to 1, otherwise, use hw pins
        bool OPMDC_1  : 1;  // Auto-negotiation enabled when all OPMDC bits are set to 1
        bool OPMDC_2  : 1;
        bool OPMDC_3  : 1;
        bool Duplex   : 1;  // Read only, Full duplex when set to 1, otherwise, half duplex
        bool Speed    : 1;  // Read only, 100Mbps when set to 1, otherwise, 10Mbps
        bool Link     : 1;  // Read only, Link up when set to 1, otherwise, link down
    } phy_t;


    typedef enum class CommonRegisterBlockAddressEnum
        : uint8_t
    {
        MODE                    = 0x00,
        GATEWAY                 = 0x01,
        SUBNETMASK              = 0x05,
        MAC                     = 0x09,
        IPv4                    = 0x0F,
        INTERRUPT_TIMER         = 0x13,
        INTERRUPT_REGISTER      = 0x15,
        INTERRUPT_MASK          = 0x16,
        SOCKET_INTERRUPT        = 0x17,
        SOCKET_INTERRUPT_MASK   = 0x18,
        RETRY_TIME              = 0x19,
        RETRY_COUNT             = 0x1B,
        UNREACHABLE_IPv4        = 0x28,
        UNREACHABLE_PORT        = 0x2C,
        PHY_CONFIGURATION       = 0x2E,
        VERSION                 = 0x39
    } crb_addr_e;


    typedef enum class SocketRegisterBlockAddressEnum
        : uint8_t
    {
        MODE                    = 0x00,
        COMMAND                 = 0x01,
        INTERRUPT_REGISTER      = 0x02,
        STATUS                  = 0x03,
        PORT_SOURCE             = 0x04, // 0x04 - 0x05
        DESTINATION_MAC         = 0x06, // 0x06 - 0x0B
        DESTINATION_IPv4        = 0x0C, // 0x0C - 0x0F
        PORT_DESTINATION        = 0x10, // 0x10 - 0x11
        MAX_SEGMENT_SIZE        = 0x12, // 0x12 - 0x13
        IP_TYPE_OF_SERVICE      = 0x15,
        IP_TIME_TO_LIVE         = 0x16,
        RX_BUFFER_SIZE          = 0x1E,
        TX_BUFFER_SIZE          = 0x1F,
        TX_FREE_SIZE            = 0x20, // 0x20 - 0x21
        TX_READ_POINTER         = 0x22, // 0x22 - 0x23
        TX_WRITE_POINTER        = 0x24, // 0x24 - 0x25
        RX_RECEIVED_SIZE        = 0x26, // 0x26 - 0x27
        RX_READ_POINTER         = 0x28, // 0x28 - 0x29
        RX_WRITE_POINTER        = 0x2A, // 0x2A - 0x2B
        INTERRUPT_MASK          = 0x2C,
        IP_FRAGMENT_OFFSET      = 0x2D, // 0x2D - 0x2E
        KEEP_ALIVE_TIMER        = 0x2F
    } srb_addr_e;


    typedef struct CommonRegisterBlockType
    {
        mr_t Mode;                       // 0x00
        uint8_t Gateway[4];              // 0x01 - 0x04
        uint8_t Subnetmask[4];           // 0x05 - 0x08
        uint8_t MAC[6];                  // 0x09 - 0x0E
        uint8_t IPv4[4];                 // 0x0F - 0x12
        uint16_t InterruptTimer;         // 0x13 - 0x14
        ir_t Interrupt;                  // 0x15
        imr_t InterruptMask;             // 0x16
        crb_sir_t SocketInterrupt;       // 0x17
        simr_t SocketInterruptMask;      // 0x18
        uint16_t RetryTime;              // 0x19 - 0x1A, Default: 2000(200ms)
        uint8_t RetryCount;              // 0x1B, Default: 8
        uint8_t UnreachableIPv4[4];      // 0x28 - 0x2B
        uint16_t UnreachablePort;        // 0x2C - 0x2D
        phy_t PhyConfiguration;          // 0x2E
        uint8_t Version;                 // 0x39, Read only, Default: 4
    } crb_t;


    typedef enum class SocketProtocolEnum
        : uint8_t 
    {
        CLOSE   = 0x00,
        TCP     = 0x01,
        UDP     = 0x02,
        MACRAW  = 0x04
    } sock_prtcl_e;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0000
     */
    typedef struct SocketModeRegisterType
    {
        bool MULTI_MFEN        : 1;  // Multicasting in UDP and MAC filter in MACRAW mode
        bool BCASTB            : 1;  // Broadcast blocking in MACRAW and UDP mode
        bool ND_MC_MMB         : 1;  // No delayed ACK in TCP, IGMP multicasting in UDP, Multicast blocking in MACRAW mode
        bool UNICASTB_MIP6B    : 1;  // Unicast blocking in UDP, IPv6 blocking in MACRAW mode
        sock_prtcl_e PROTOCOL  : 4;  // 0b0000 for closed, 0b0001 for TCP, 0b0010 for UDP, 0b0100 for MACRAW for Socket 0
    } smr_t;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0001
     * 
     * @details 
     *   - W5500 will automatically clears the command after execution.
     *   - The execution results must be checked through status register.
     */
    typedef enum class SocketCommandRegisterEnum
        : uint8_t
    {
        OPEN              = 0x01,
        LISTEN            = 0x02,   // TCP mode only
        CONNECT           = 0x04,   // TCP mode only
        DISCONNECT        = 0x08,   // TCP mode only
        CLOSE             = 0x10,
        SEND              = 0x20,
        SEND_MAC          = 0x21,   // UDP mode only
        SEND_KEEPALIVE    = 0x22,   // TCP mode only
        RECEIVE           = 0x40
    } scr_e;

    
    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0002
     * 
     * @details 
     *   - Write 1 to the bit to clear the interrupt.
     */
    typedef struct SocketInterruptRegisterType
    {
        bool SEND_OK        : 1;  // [4]
        bool TIMEOUT        : 1;  // [3] ARP timeout or TCP timeout
        bool RECEIVED       : 1;  // [2]
        bool DISCONNECTED   : 1;  // [1]
        bool CONNECTED      : 1;  // [0] socket status is set to ESTABLISHED
    } sir_t;

    typedef enum class SocketInterruptRegisterEnum
        : uint8_t
    {
        SEND_OK       = 0x04,  // [4]
        TIMEOUT       = 0x03,  // [3] ARP timeout or TCP timeout
        RECEIVED      = 0x02,  // [2]
        DISCONNECTED  = 0x01,  // [1]
        CONNECTED     = 0x00,  // [0] socket status is set to ESTABLISHED
    } sir_e;


    /**
     * @brief
     *   - Access Mode: Read, Write
     *   - Default Value: 0x00
     *   - Address: 0x0003
     * 
     * @details 
     *   - Status can be vary by the command or communication process.
     *   - Need to check the status before executing commands.
     */
    typedef enum class SocketStatusRegisterEnum
        : uint8_t
    {
        CLOSED          = 0x00,  // Triggered by DISCONNECT, CLOSE command or ARP, TCP timeout
        INIT_TCP        = 0x13,  // Socket is opend as a TCP mode. LISTEN or CONNECT command can be used
        LISTEN          = 0x14,  // The socket is working as a TCP server and waiting for SYN request.
        SYN_SENT        = 0x15,  // Sent SYN packet to the remote side.
        SYN_RECV        = 0x16,  // Received SYN packet from the remote side.
        ESTABLISHED     = 0x17,  // TCP connection has been made after SYN request processed successfully.
        FIN_WAIT        = 0x18,  // Waiting for the FIN packet from the remote side after sening FIN packet.
        CLOSING         = 0x1A,  // Waiting for the ACK packet to the FIN you sent.
        TIME_WAIT       = 0x1B,  // Waiting for remained packets after TCP connection is closed to make sure new connection is not interfered by the old one.
        CLOSE_WAIT      = 0x1C,  // FIN packet received. DISCONNEC or CLOSE command can be used.
        LAST_ACK        = 0x1D,  // Waiting for remote side's ACK packet to the the FIN which W5500 sent.
        INIT_UDP        = 0x22,  // Socket is opend as a UDP mode.
        INIT_MACRAW     = 0x42,  // Socket is opend as a MACRAW mode. Direct access to Ethernet frames available.
        UNCERTAIN       = 0xFF
    } ssr_e;


    typedef struct SocketRegisterBlockType
    {
        smr_t Mode;                      // 0x00
        scr_e Command;                   // 0x01
        sir_t Interrupt;                 // 0x02
        ssr_e Status;                    // 0x03
        uint16_t SourcePort;             // 0x04 - 0x05, set before OPEN command and it only works in TCP, UDP modes
        uint8_t DestinationMAC[4];       // 0x06 - 0x0B
        uint8_t DestinationIPv4[4];      // 0x0C - 0x0F, set before CONNECT command for TCP client, SEND command for UDP client and it only works in TCP, UDP modes
        uint16_t DestinationPort;        // 0x10 - 0x11, set before CONNECT command for TCP client, SEND command for UDP client and it only works in TCP, UDP modes
        uint16_t MaxSegmentSize;         // 0x12 - 0x13
        uint8_t TOS;                     // 0x15, type of service
        uint8_t TTL;                     // 0x16, time to live
        uint8_t RxBufferSize;            // 0x1E
        uint8_t TxBufferSize;            // 0x1F
        uint16_t TxFreeSize;             // 0x20 - 0x21
        uint16_t TxReadPointer;          // 0x22 - 0x23
        uint16_t TxWritePointer;         // 0x24 - 0x25
        uint16_t RxReceivedSize;         // 0x26 - 0x27
        uint16_t RxReadPointer;          // 0x28 - 0x29
        uint16_t RxWritePointer;         // 0x2A - 0x2B
        uint8_t InterruptMask;           // 0x2C
        uint16_t FragmentOffset;         // 0x2D - 0x2E
        uint8_t KeepAliveTimer;          // 0x2F
    } srb_t;


    typedef enum class InternetProtocolAddressTypeEnum
    {
        SOURCE_IPv4,
        GATEWAY,
        SUBNETMASK
    } ipv4_type_e;
}}