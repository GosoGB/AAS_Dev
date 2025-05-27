/**
 * @file Converter.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */




#pragma once

#include <sys/_stdint.h>

#include "Common/Assert.h"
#include "TypeDefinitions.h"



namespace muffin { namespace w5500 {

    
    class Converter
    {
    public:
        static uint8_t ControlPhase(const sock_id_e socket, const am_e access)
        {
            bsb_e block = bsb_e::COMMON_REGISTER;

            switch (socket)
            {
            case sock_id_e::SOCKET_0:
                block = bsb_e::SOCKET_0_REGISTER;
                break;

            case sock_id_e::SOCKET_1:
                block = bsb_e::SOCKET_1_REGISTER;
                break;

            case sock_id_e::SOCKET_2:
                block = bsb_e::SOCKET_2_REGISTER;
                break;

            case sock_id_e::SOCKET_3:
                block = bsb_e::SOCKET_3_REGISTER;
                break;

            case sock_id_e::SOCKET_4:
                block = bsb_e::SOCKET_4_REGISTER;
                break;

            case sock_id_e::SOCKET_5:
                block = bsb_e::SOCKET_5_REGISTER;
                break;

            case sock_id_e::SOCKET_6:
                block = bsb_e::SOCKET_6_REGISTER;
                break;

            case sock_id_e::SOCKET_7:
                block = bsb_e::SOCKET_7_REGISTER;
                break;

            default:
                ASSERT(false, "INVALID SOCKET INDEX");
                break;
            }

            return ControlPhase(block, access);
        }


        static uint8_t ControlPhase(const bsb_e block, const am_e access)
        {
            const uint8_t blockBits   = static_cast<uint8_t>(block)  << 3;
            const uint8_t accessBits  = static_cast<uint8_t>(access) << 2;
            const uint8_t modeBits    = 0x00;  // 하드웨어상으로 고정된 값임
            return (blockBits | accessBits | modeBits);
        }


        static uint8_t ModeRegister(const mr_t reg)
        {
            return (reg.RST << 7 | reg.WoL << 5 | reg.PB << 4 | reg.PPPoE << 3 | reg.FARP << 1);
        }


        static uint8_t ToUint8(const smr_t mode)
        {
            uint8_t value = 0;

            value |= mode.MULTI_MFEN       ? 1 << 7 : 0 << 7;
            value |= mode.BCASTB           ? 1 << 6 : 0 << 6;
            value |= mode.ND_MC_MMB        ? 1 << 5 : 0 << 5;
            value |= mode.UNICASTB_MIP6B   ? 1 << 4 : 0 << 4;
            value |= static_cast<uint8_t>(mode.PROTOCOL);

            return  value;
        }


        static smr_t ToSocketMode(const uint8_t mode)
        {
            smr_t value;

            value.MULTI_MFEN       = (mode >> 7) & 0x01;
            value.BCASTB           = (mode >> 6) & 0x01;
            value.ND_MC_MMB        = (mode >> 5) & 0x01;
            value.UNICASTB_MIP6B   = (mode >> 4) & 0x01;
            value.PROTOCOL         = static_cast<sock_prtcl_e>(mode & 0x0F);
            
            return value;
        }


        static const char* ToString(const ssr_e status)
        {
            switch (status)
            {
            case ssr_e::CLOSED:
                return "CLOSED";
            case ssr_e::INIT_TCP:
                return "INIT_TCP";
            case ssr_e::LISTEN:
                return "LISTEN";
            case ssr_e::SYN_SENT:
                return "SYN_SENT";
            case ssr_e::SYN_RECV:
                return "SYN_RECV";
            case ssr_e::ESTABLISHED:
                return "ESTABLISHED";
            case ssr_e::FIN_WAIT:
                return "FIN_WAIT";
            case ssr_e::CLOSING:
                return "CLOSING";
            case ssr_e::TIME_WAIT:
                return "TIME_WAIT";
            case ssr_e::CLOSE_WAIT:
                return "CLOSE_WAIT";
            case ssr_e::LAST_ACK:
                return "LAST_ACK";
            case ssr_e::INIT_UDP:
                return "INIT_UDP";
            case ssr_e::INIT_MACRAW:
                return "INIT_MACRAW";
            default:
                return "UNCERTAIN";
            }
        }
    };
}}