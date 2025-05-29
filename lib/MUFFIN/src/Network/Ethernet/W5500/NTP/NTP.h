// #if defined(MT11)

// /**
//  * @file NTP.h
//  * @author Lee, Sang-jin (lsj31@edgecross.ai)
//  * 
//  * @brief 
//  * 
//  * @date 2025-05-29
//  * @version 1.4.0
//  * 
//  * @copyright Copyright (c) Edgecross Inc. 2025
//  */




// #pragma once

// #include <sys/_stdint.h>

// #include "Common/Status.h"
// #include "Network/Ethernet/W5500/Socket.h"



// namespace muffin { namespace w5500 {


//     typedef struct NtpDataFormatType
//     {
//         uint8_t    dstaddr[4];     /* destination (local) address */
//         int8_t     version;        /* version number */
//         int8_t     leap;           /* leap indicator */
//         int8_t     mode;           /* mode */
//         int8_t     stratum;        /* stratum */
//         int8_t     poll;           /* poll interval */
//         uint8_t    precision;      /* precision */
//         uint32_t   rootdelay;      /* root delay */
//         uint32_t   rootdisp;       /* root dispersion */
//         int8_t     refid;          /* reference ID */
//         uint64_t   reftime;        /* reference time */
//         uint64_t   org;            /* origin timestamp */
//         uint64_t   rec;            /* receive timestamp */
//         uint64_t   xmt;            /* transmit timestamp */
//     } ntp_fmt_t;


//     typedef struct NtpDatetimeType
//     {
//         uint16_t yy;
//         uint8_t mo;
//         uint8_t dd;
//         uint8_t hh;
//         uint8_t mm;
//         uint8_t ss;
//     } datetime_t;
    
//     class NTP
//     {
//     public:
//         NTP() {}
//         ~NTP() {}
//     public:
//         void Init(Socket& socket, IPAddress host, uint8_t tz, uint8_t* buf);
//         Status Fetch(datetime_t* output);
//     private:
//         void calculateDatetime(const uint64_t seconds);
//         void fetchSeconds(uint16_t idx, uint8_t* buf);
//         uint64_t changeDatetime2Seconds();
//     private:
//         Socket* mSocket = nullptr;
//         ntp_fmt_t mNtpData;
//         uint8_t mTimezone;
//         uint8_t mNtpMessage[48];
//     };
    

    
//     void SNTP_init(uint8_t s, uint8_t *ntp_server, uint8_t tz, uint8_t *buf);
//     int8_t SNTP_run(datetime *time);
    
// }}


// #endif