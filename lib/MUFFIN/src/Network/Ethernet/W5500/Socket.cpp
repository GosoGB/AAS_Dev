#if defined(MT11)

/**
 * @file Sockets.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-08
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#include <sys/_stdint.h>

#include "Common/Assert.h"
#include "Socket.h"
#include "W5500.h"
#include "Include/TypeDefinitions.h"


namespace muffin { namespace w5500 {


    Socket::Socket(W5500& interface, const sock_id_e idx, const sock_prtcl_e protocol)
        : mW5500(interface)
        , mID(idx)
        , mProtocol(protocol)
    {
        ASSERT((static_cast<uint8_t>(idx) < static_cast<uint8_t>(sock_id_e::TOP)),
            "INVALID SOCKET ID: %d", static_cast<uint8_t>(idx));
        
        memset(&mSRB, 0, sizeof(mSRB));
        memset(&mSRB.DestinationMAC, 0xFF, sizeof(mSRB.DestinationMAC));
        mSRB.TTL             = 0x0080;
        mSRB.RxBufferSize    = 0x0002;
        mSRB.TxBufferSize    = 0x0002;
        mSRB.TxFreeSize      = 0x0800;
        mSRB.InterruptMask   = 0x00FF;
        mSRB.FragmentOffset  = 0x4000;
        // LOG_DEBUG(logger,"SOCKET 생성, ID : %d",static_cast<uint8_t>(mID));
        if (mID != sock_id_e::SOCKET_0)
        {
            mW5500.SetSocketIdFlag(mID);
        }
        
    }


    Socket::~Socket()
    {
        for (uint8_t trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
        {
            Status ret = Close();
            if (ret != Status::Code::GOOD)
            {
                LOG_WARNING(logger, "[#%u] SOCKET IS NOT CLOSED: %s", static_cast<uint8_t>(mID), ret.c_str());
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            else
            {
                break;
            }
        }
        mW5500.ResetSocketIdFlag(mID);
    }


    ssr_e Socket::GetStatus()
    {
        do
        {
            uint8_t status;
            Status ret = mW5500.retrieveSRB(mID, srb_addr_e::STATUS, &status);;
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO READ SOCKET STATUS: %s", ret.c_str());
                continue;
            }
            
            switch (status)
            {
            case static_cast<uint8_t>(ssr_e::CLOSED):
            case static_cast<uint8_t>(ssr_e::INIT_TCP):
            case static_cast<uint8_t>(ssr_e::LISTEN):
            case static_cast<uint8_t>(ssr_e::SYN_SENT):
            case static_cast<uint8_t>(ssr_e::SYN_RECV):
            case static_cast<uint8_t>(ssr_e::ESTABLISHED):
            case static_cast<uint8_t>(ssr_e::FIN_WAIT):
            case static_cast<uint8_t>(ssr_e::CLOSING):
            case static_cast<uint8_t>(ssr_e::TIME_WAIT):
            case static_cast<uint8_t>(ssr_e::CLOSE_WAIT):
            case static_cast<uint8_t>(ssr_e::LAST_ACK):
            case static_cast<uint8_t>(ssr_e::INIT_UDP):
            case static_cast<uint8_t>(ssr_e::INIT_MACRAW):
                mSRB.Status = static_cast<ssr_e>(status);
                // LOG_DEBUG(logger, "Socket Status: %s", Converter::ToString(mSRB.Status));
                break;
            default:
                mSRB.Status = ssr_e::UNCERTAIN;
                LOG_WARNING(logger, "SOCKET STATUS: %s", Converter::ToString(mSRB.Status));
                break;
            }
            
        } while (mSRB.Status == ssr_e::UNCERTAIN);

        return mSRB.Status;
    }


    Status Socket::GetInterrupt(sir_t* interrupt)
    {
        ASSERT(interrupt != nullptr, "INVALID ARGUMENT: NULL POINTER");

        uint8_t retrievedInterrupt = 0x00;
        Status ret = mW5500.retrieveSRB(mID, srb_addr_e::INTERRUPT_REGISTER, &retrievedInterrupt);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SOCKET INTERRUPT: %s", ret.c_str());
            return ret;
        }
        
        /**
         * @todo interrupt 풀어주는 거에 대해 고민 필요함
         */
        mSRB.Interrupt.SEND_OK       = (retrievedInterrupt >> 4) & 0x01;
        mSRB.Interrupt.TIMEOUT       = (retrievedInterrupt >> 3) & 0x01;
        mSRB.Interrupt.RECEIVED      = (retrievedInterrupt >> 2) & 0x01;
        mSRB.Interrupt.DISCONNECTED  = (retrievedInterrupt >> 1) & 0x01;
        mSRB.Interrupt.CONNECTED     = (retrievedInterrupt >> 0) & 0x01;

        *interrupt = mSRB.Interrupt;
        return ret;
    }


    sock_id_e Socket::GetSocketID() const
    {
        return mID;
    }


    sock_prtcl_e Socket::GetProtocol() const
    {
        return mProtocol;
    }

    
    uint16_t Socket::Available()
    {
        Status ret = retrieveReceivedSize();
        if (ret != Status::Code::GOOD)
        {
            return 0;
        }
        else
        {
            return mSRB.RxReceivedSize;
        }
    }


    Status Socket::Open()
    {
        const uint16_t ephemeralPort = mW5500.GetEphemeralPort();
        return openInternal(ephemeralPort);
    }

    Status Socket::Open(const uint16_t port)
    {
        return openInternal(port);
    }

    Status Socket::openInternal(const uint16_t port)
    {
        // configure socket mode
        switch (mProtocol)
        {
        case sock_prtcl_e::TCP:
            mSRB.Mode.ND_MC_MMB  = true;
            mSRB.Mode.PROTOCOL   = mProtocol;
            break;
        case sock_prtcl_e::UDP:
            mSRB.Mode.PROTOCOL   = mProtocol;
            break;
        case sock_prtcl_e::MACRAW:
            return Status(Status::Code::BAD_NOT_IMPLEMENTED);
        default:
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        const uint8_t targetMode = Converter::ToUint8(mSRB.Mode);
        Status ret = mW5500.writeSRB(mID, srb_addr_e::MODE, targetMode);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE SOCKET MODE: %s", ret.c_str());
            return ret;
        }
        
        uint8_t retrievedMode = 0;
        ret = mW5500.retrieveSRB(mID, srb_addr_e::MODE, &retrievedMode);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SOCKET MODE: %s", ret.c_str());
            return ret;
        }

        if (targetMode != retrievedMode)
        {
            LOG_ERROR(logger, "FAILED TO SET MODE: '0x%02X' != '0x%02X'", targetMode, retrievedMode);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        // configure port number
        ret = setSourcePort(port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET SOURCE PORT: %s", ret.c_str());
            return ret;
        }
        
        // send command 'OPEN'
        uint8_t command = static_cast<uint8_t>(scr_e::OPEN);
        ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN SOCKET: %s", ret.c_str());
            return ret;
        }
        
        waitCommandCleared();
        GetStatus();
      
        if ((mProtocol == sock_prtcl_e::TCP    && mSRB.Status != ssr_e::INIT_TCP) ||
            (mProtocol == sock_prtcl_e::UDP    && mSRB.Status != ssr_e::INIT_UDP) ||
            (mProtocol == sock_prtcl_e::MACRAW && mSRB.Status != ssr_e::INIT_MACRAW))
        {
            LOG_ERROR(logger, "FAILED TO OPEN SOCKET: 0x%02X", static_cast<uint8_t>(mSRB.Status));
            return Status(Status::Code::BAD);
        }

        // LOG_DEBUG(logger, "[#%u] The socket has been opened", static_cast<uint8_t>(mID));
        return ret;
    }


    Status Socket::Close()
    {
        uint8_t command = static_cast<uint8_t>(scr_e::CLOSE);
        Status ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE SOCKET: %s", ret.c_str());
            return ret;
        }
        
        waitCommandCleared();
        
        uint8_t trialCount = 0;
        for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
        {
            GetStatus();
            if (mSRB.Status == ssr_e::CLOSED)
            {
                // LOG_DEBUG(logger, "[#%u] The socket has been closed", static_cast<uint8_t>(mID));
                break;
            }
            else
            {
                LOG_WARNING(logger, "[#%u] SOCKET NOT CLOSED: %s", static_cast<uint8_t>(mID), Converter::ToString(mSRB.Status));
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        
        if (trialCount == MAX_TRIAL_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE THE SOCKET #%02u", static_cast<uint8_t>(mID));
            return Status(Status::Code::BAD);
        }
        
        /**
         * @brief Release the sock_io_mode of SOCKETn.
         * @code {.cpp}
         *      sock_io_mode &= ~(1<<sn); 
         *      sock_remained_size[sn] = 0;
         *      sock_is_sending &= ~(1<<sn);
         *      sock_pack_info[sn] = PACK_NONE;
         * @endcode
         */

        clearInterrupt();        
        return ret;
    }


    Status Socket::Bind(const uint16_t port)
    {
        ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::CLOSE), "SOCKET MUST BE UNOPENED MODE");
        ASSERT((mSRB.Status == ssr_e::CLOSED), "SOCKET MUST BE CLOSED");
        ASSERT((port != 0), "INVALID PORT NUMBER: %u", port);

        Status ret = mW5500.writeSRB(mID, srb_addr_e::PORT_SOURCE, port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE SOURCE PORT: %s", ret.c_str());
            return ret;
        }
        
        uint8_t retrievedPort = 0;
        ret = mW5500.retrieveSRB(mID, srb_addr_e::PORT_SOURCE, &retrievedPort);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE SOURCE PORT: %s", ret.c_str());
            return ret;
        }

        if (port != retrievedPort)
        {
            LOG_ERROR(logger, "FAILED TO SOURCE PORT: '%u' != '%u'", port, retrievedPort);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        mSRB.SourcePort = port;
        return ret;
    }


    Status Socket::Listen()
    {
        LOG_ERROR(logger, "SOCKET LISTEN NOT IMPLEMENTED");
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status Socket::Accept(IPAddress* ip, uint16_t* port)
    {
        LOG_ERROR(logger, "SOCKET ACCEPT NOT IMPLEMENTED");
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status Socket::Connect(const IPAddress ip, const uint16_t port)
    {
        ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::TCP), "SOCKET IS NOT IN TCP MODE");
        ASSERT((mSRB.Status == ssr_e::INIT_TCP), "SOCKET IS NOT INITIALIZED FOR TCP MODE");
        ASSERT((uint32_t(ip) != 0xFFFFFFFF), "[#%u] INVALID HOST TCP IP ADDRESS: %s", static_cast<uint8_t>(mID), ip.toString().c_str());
        
        Status ret = setHostIPv4(ip);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        ret = setHostPort(port);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        // execute command 'CONNECT'
        ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, static_cast<uint8_t>(scr_e::CONNECT));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT SOCKET: %s", ret.c_str());
            return ret;
        }
        
        waitCommandCleared();
        waitInterruptEvnet();
        GetStatus();

        if (mSRB.Interrupt.TIMEOUT == true)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT DUE TO TIMEOUT: SOCKET #%u", static_cast<uint8_t>(mID));
            ret = Status::Code::BAD_TIMEOUT;
        }
        else if ((mSRB.Interrupt.CONNECTED == false) || (mSRB.Status != ssr_e::ESTABLISHED))
        {
            LOG_ERROR(logger, "FAILED TO CONNECT: SOCKET #%u", static_cast<uint8_t>(mID));
            ret = Status::Code::BAD_CONNECTION_REJECTED;
        }

        clearInterrupt();
        return ret;
    }


    Status Socket::Disconnect()
    {
        uint8_t command = static_cast<uint8_t>(scr_e::DISCONNECT);
        Status ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DISCONNECT SOCKET: %s", ret.c_str());
            return ret;
        }
        
        uint8_t trialCount = 0;
        for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
        {
            GetStatus();
            if (mSRB.Status == ssr_e::CLOSED)
            {
                // LOG_DEBUG(logger, "[#%u] The socket has been disconnected", static_cast<uint8_t>(mID));
                break;
            }
            else
            {
                // LOG_DEBUG(logger, "[#%u] SOCKET NOT DISCONNECT: %s", static_cast<uint8_t>(mID), Converter::ToString(mSRB.Status));
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        
        if (trialCount == MAX_TRIAL_COUNT)
        {
            LOG_ERROR(logger, "FAILED TO DISCONNECT THE SOCKET #%02u", static_cast<uint8_t>(mID));
            return Status(Status::Code::BAD);
        }
        
        /**
         * @brief Release the sock_io_mode of SOCKETn.
         * @code {.cpp}
         *      sock_io_mode &= ~(1<<sn); 
         *      sock_remained_size[sn] = 0;
         *      sock_is_sending &= ~(1<<sn);
         *      sock_pack_info[sn] = PACK_NONE;
         * @endcode
         */

        clearInterrupt();        
        return ret;
    }


    Status Socket::Send(const size_t length, const uint8_t data[])
    {
        ASSERT((mSRB.Status == ssr_e::ESTABLISHED), "SOCKET IS NOT INITIALIZED FOR UDP MODE");
        ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::TCP), "SOCKET IS NOT IN UDP MODE");
        ASSERT((length > 0), "INVALID DATA LENGTH: %u", length);
        ASSERT((data != nullptr), "INVALID DATA POINTER");
        
        Status ret(Status::Code::UNCERTAIN);
        size_t remainedLength = length;
        size_t sentLength = 0;
        
        while (remainedLength > 0)
        {
            GetStatus();
            if (mSRB.Status == ssr_e::CLOSED)
            {
                LOG_ERROR(logger, "SOCKET #%u HAS BEEN CLOSED", static_cast<uint8_t>(mID));
                return Status(Status::Code::BAD_CONNECTION_CLOSED);
            }
            
            ret = retrieveTxFreeSize();
            if (ret != Status::Code::GOOD)
            {
                continue;
            }
            
            const uint16_t lengthToSend = remainedLength > mSRB.TxFreeSize ? mSRB.TxFreeSize : remainedLength;
            ret = implementSend(lengthToSend, data);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "[#%u] FAILED TO SEND DATA: %s", static_cast<uint8_t>(mID), ret.c_str());
                return ret;
            }
            
            remainedLength  -= lengthToSend;
            sentLength      += lengthToSend;
            data            += lengthToSend;
        }
        
        return ret;
    }


    Status Socket::Receive(const size_t length, size_t* actualLength, uint8_t* data)
    {
        // ASSERT((mSRB.Status == ssr_e::LISTEN), "SOCKET IS NOT IN LISTEN MODE(TCP/IP)");
        // ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::TCP), "SOCKET IS NOT IN TCP MODE");
        // ASSERT((length > 0), "INVALID BUFFER LENGTH TO STORE Rx DATA: %u", length);
        // ASSERT((actualLength != nullptr), "INVALID RECEIVED LENGTH POINTER");
        // ASSERT((data != nullptr), "INVALID DATA POINTER");
        
        GetStatus();
        if (mSRB.Status == ssr_e::CLOSED)
        {
            LOG_ERROR(logger, "SOCKET #%u HAS BEEN CLOSED", static_cast<uint8_t>(mID));
            return Status(Status::Code::BAD_CONNECTION_CLOSED);
        }

        Status ret = retrieveReceivedSize();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE RECEIVED SIZE: %s", ret.c_str());
            return ret;
        }

        const uint16_t lengthToRead = mSRB.RxReceivedSize > length ? length : mSRB.RxReceivedSize;
        *actualLength = 0;
        // LOG_DEBUG(logger, "lengthToRead: %u", lengthToRead);
        ret = implementReceive(lengthToRead, data);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "[#%u] FAILED TO SEND DATA: %s", static_cast<uint8_t>(mID), ret.c_str());
            return ret;
        }
        
        *actualLength = lengthToRead;
        return ret;
    }


    Status Socket::SendTo(const IPAddress ip, const uint16_t port, const size_t length, const uint8_t data[])
    {
        ASSERT((mSRB.Status == ssr_e::INIT_UDP), "SOCKET IS NOT INITIALIZED FOR UDP MODE");
        ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::UDP), "SOCKET IS NOT IN UDP MODE");
        ASSERT((length > 0), "INVALID DATA LENGTH: %u", length);
        ASSERT((data != nullptr), "INVALID DATA POINTER");
        
        Status ret = setHostIPv4(ip);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        ret = setHostPort(port);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        size_t remainedLength = length;
        size_t sentLength = 0;
        
        while (remainedLength > 0)
        {
            ret = retrieveTxFreeSize();
            if (ret != Status::Code::GOOD)
            {
                continue;
            }
            
            const uint16_t lengthToSend = remainedLength > mSRB.TxFreeSize ? mSRB.TxFreeSize : remainedLength;
            ret = implementSend(lengthToSend, data);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "[#%u] FAILED TO SEND DATA: %s", static_cast<uint8_t>(mID), ret.c_str());
                return ret;
            }
            
            remainedLength  -= lengthToSend;
            sentLength      += lengthToSend;
            data            += lengthToSend;
        }
        
        return ret;
    }


    Status Socket::ReceiveFrom(const IPAddress ip, const uint16_t port, const size_t length, size_t* actualLength, uint8_t* data)
    {
        ASSERT((mSRB.Status == ssr_e::INIT_UDP), "SOCKET IS NOT INITIALIZED FOR UDP MODE");
        ASSERT((mSRB.Mode.PROTOCOL == sock_prtcl_e::UDP), "SOCKET IS NOT IN UDP MODE");
        ASSERT((length > 0), "INVALID BUFFER LENGTH TO STORE Rx DATA: %u", length);
        ASSERT((actualLength != nullptr), "INVALID RECEIVED LENGTH POINTER");
        ASSERT((data != nullptr), "INVALID DATA POINTER");

        Status ret = retrieveReceivedSize();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE RECEIVED SIZE: %s", ret.c_str());
            return ret;
        }

        const uint16_t lengthToRead = mSRB.RxReceivedSize > length ? length : mSRB.RxReceivedSize;
        *actualLength = 0;
        // LOG_DEBUG(logger, "lengthToRead: %u", lengthToRead);
        ret = implementReceive(lengthToRead, data);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "[#%u] FAILED TO SEND DATA: %s", static_cast<uint8_t>(mID), ret.c_str());
            return ret;
        }
        
        *actualLength = lengthToRead;
        return ret;
    }


    void Socket::waitCommandCleared()
    {
        Status ret(Status::Code::UNCERTAIN);
        uint8_t retrievedCommand = 0xFF;
        
        while (retrievedCommand != 0)
        {
            ret = mW5500.retrieveSRB(mID, srb_addr_e::COMMAND, &retrievedCommand);
            if (ret != Status::Code::GOOD)
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
                continue;
            }
            // LOG_DEBUG(logger, "Retrieved command: 0x%02X", retrievedCommand);
        }
    }


    void Socket::waitInterruptEvnet()
    {
        Status ret(Status::Code::UNCERTAIN);
        uint8_t retrievedInterrupt = 0x00;
        
        while (retrievedInterrupt == 0)
        {
        #if defined(DEBUG)
        #endif
        vTaskDelay(10 / portTICK_PERIOD_MS);
        GetStatus();
            ret = mW5500.retrieveSRB(mID, srb_addr_e::INTERRUPT_REGISTER, &retrievedInterrupt);
            if (ret != Status::Code::GOOD)
            {
                continue;
            }
            // LOG_DEBUG(logger, "Retrieved interrupt: 0x%02X", retrievedInterrupt);
        }

        mSRB.Interrupt.SEND_OK       = (retrievedInterrupt >> 4) & 0x01;
        mSRB.Interrupt.TIMEOUT       = (retrievedInterrupt >> 3) & 0x01;
        mSRB.Interrupt.RECEIVED      = (retrievedInterrupt >> 2) & 0x01;
        mSRB.Interrupt.DISCONNECTED  = (retrievedInterrupt >> 1) & 0x01;
        mSRB.Interrupt.CONNECTED     = (retrievedInterrupt >> 0) & 0x01;
    }


    void Socket::clearInterrupt()
    {
        while (true)
        {
            const uint8_t command = 0b00011111;
            Status ret = mW5500.writeSRB(mID, srb_addr_e::INTERRUPT_REGISTER, command);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CLEAR INTERRUPT REGISTER: %s", ret.c_str());
                continue;
            }

            uint8_t retrievedInterrupt = 0x00;
            ret = mW5500.retrieveSRB(mID, srb_addr_e::INTERRUPT_REGISTER, &retrievedInterrupt);
            if (retrievedInterrupt != 0x00)
            {
                LOG_ERROR(logger, "FAILED TO CLEAR INTERRUPT REGISTER: %s", ret.c_str());
                continue;
            }
            else
            {
                mSRB.Interrupt.SEND_OK       = 0x00;
                mSRB.Interrupt.TIMEOUT       = 0x00;
                mSRB.Interrupt.RECEIVED      = 0x00;
                mSRB.Interrupt.DISCONNECTED  = 0x00;
                mSRB.Interrupt.CONNECTED     = 0x00;
                break;
            }
        }
    }


    Status Socket::setHostIPv4(const IPAddress ip)
    {
        ASSERT((ip != INADDR_NONE), "[#%u] INVALID HOST IP ADDRESS: %s", static_cast<uint8_t>(mID), ip.toString().c_str());

        uint8_t targetIP[4] = { 0 };
        const uint32_t numericIP = uint32_t(ip);
        memcpy(targetIP, &numericIP, sizeof(targetIP));
        Status ret = mW5500.writeSRB(mID, srb_addr_e::DESTINATION_IPv4, sizeof(targetIP), targetIP);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE HOST IP ADDRESS: %s", ret.c_str());
            return ret;
        }

        /** Connect 같은 명령 이후에나 확인 가능함
            uint8_t retrievedIP[4] = { 0 };
            ret = mW5500.retrieveSRB(mID, srb_addr_e::DESTINATION_IPv4, sizeof(retrievedIP), retrievedIP);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE HOST IPv4: %s", ret.c_str());
                return ret;
            }
            
            if (memcmp(targetIP, retrievedIP, sizeof(targetIP)) != 0)
            {
                LOG_ERROR(logger, "FAILED TO SET HOST IP ADDRESS: '%u.%u.%u.%u' != '%s'", retrievedIP[0],
                                                                                        retrievedIP[1],
                                                                                        retrievedIP[2],
                                                                                        retrievedIP[3],
                                                                                        ip.toString().c_str());
                return ret;
            }
            memcpy(mSRB.DestinationIPv4, &numericIP, sizeof(mSRB.DestinationIPv4));
         */

        return ret;
    }


    Status Socket::setHostPort(const uint16_t port)
    {
        ASSERT((port != 0), "[#%u] INVALID HOST PORT NUMBER: %u", static_cast<uint8_t>(mID), port);

        Status ret = mW5500.writeSRB(mID, srb_addr_e::PORT_DESTINATION, port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE HOST PORT: %s", ret.c_str());
            return ret;
        }
        
    /** Connect 같은 명령 이후에나 확인 가능함
        uint8_t portRead = 0;
        ret = mW5500.retrieveSRB(mID, srb_addr_e::PORT_DESTINATION, &portRead);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE HOST PORT: %s", ret.c_str());
            return ret;
        }
        
        if (port != portRead)
        {
            LOG_ERROR(logger, "FAILED TO SET HOST PORT: '%u' != '%u'", mSRB.DestinationPort, portRead);
            return ret;
        }

        mSRB.DestinationPort = port;
    */

        return ret;
    }


    Status Socket::setSourcePort(const uint16_t port)
    {
        ASSERT((port != 0), "[#%u] INVALID HOST PORT NUMBER: %u", static_cast<uint8_t>(mID), port);

        Status ret = mW5500.writeSRB(mID, srb_addr_e::PORT_SOURCE, port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE HOST PORT: %s", ret.c_str());
            return ret;
        }
        
        uint16_t portRead = 0;
        ret = mW5500.retrieveSRB(mID, srb_addr_e::PORT_SOURCE, &portRead);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE HOST PORT: %s", ret.c_str());
            return ret;
        }
        
        if (port != portRead)
        {
            LOG_ERROR(logger, "FAILED TO SET HOST PORT: '%u' != '%u'", port, portRead);
            return ret;
        }

        mSRB.SourcePort = port;
        // LOG_DEBUG(logger, "SOCKET #%u SOURCE PORT: %u", static_cast<uint8_t>(mID), mSRB.SourcePort);
        return ret;
    }


    Status Socket::retrieveTxFreeSize()
    {
        uint16_t txFreeSize = UINT16_MAX;
        Status ret = mW5500.retrieveSRB(mID, srb_addr_e::TX_FREE_SIZE, &txFreeSize);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE TX BUFFER FREE SIZE: %s", ret.c_str());
            return ret;
        }

        mSRB.TxFreeSize = txFreeSize;
        return ret;
    }


    Status Socket::retrieveReceivedSize()
    {
        uint16_t receivedSize = UINT16_MAX;
        Status ret = mW5500.retrieveSRB(mID, srb_addr_e::RX_RECEIVED_SIZE, &receivedSize);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE RECEIVED SIZE: %s", ret.c_str());
            return ret;
        }

        mSRB.RxReceivedSize = receivedSize;
        return ret;
    }


    Status Socket::implementSend(const size_t length, const uint8_t data[])
    {
        // retrieve Tx buffer write pointer
        uint16_t writePointer = UINT16_MAX;
        Status ret = mW5500.retrieveSRB(mID, srb_addr_e::TX_WRITE_POINTER, &writePointer);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE TX WRITE POINTER: %s", ret.c_str());
            return ret;
        }
        // LOG_DEBUG(logger, "TX WRITE POINTER: %u", writePointer);
        
        // write data to the Tx buffer
        const bsb_e bsb = static_cast<bsb_e>(4*static_cast<uint8_t>(mID) + 0x02);
        const uint8_t controlPhase  = Converter::ControlPhase(bsb, am_e::WRITE);
        ret = mW5500.write(writePointer, controlPhase, length, data);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO WRITE TX BUFFER: %s", ret.c_str());
            return ret;
        }

        writePointer += length;
        ret = mW5500.writeSRB(mID, srb_addr_e::TX_WRITE_POINTER, writePointer);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE TX WRITE POINTER: %s", ret.c_str());
            return ret;
        }
        
        // send data stored in the Tx buffer
        ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, static_cast<uint8_t>(scr_e::SEND));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SEND DATA: %s", ret.c_str());
            return ret;
        }

        // check the result of sending
        waitCommandCleared();
        waitInterruptEvnet();
        GetStatus();

        if (mSRB.Interrupt.TIMEOUT == true)
        {
            LOG_ERROR(logger, "FAILED TO SEND DATA DUE TO TIMEOUT: SOCKET #%u", static_cast<uint8_t>(mID));
            ret = Status::Code::BAD_TIMEOUT;
        }
        else if (mSRB.Interrupt.DISCONNECTED == true)
        {
            LOG_ERROR(logger, "FAILED TO SEND DATA DUE TO DISCONNECTION: SOCKET #%u", static_cast<uint8_t>(mID));
            ret = Status::Code::BAD_CONNECTION_CLOSED;
        }
        
        clearInterrupt();
        return ret;
    }


    Status Socket::implementReceive(const size_t length, uint8_t* data)
    {
        // retrieve Rx buffer read pointer
        uint16_t readPointer = UINT16_MAX;
        Status ret = mW5500.retrieveSRB(mID, srb_addr_e::RX_READ_POINTER, &readPointer);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE RX READ POINTER: %s", ret.c_str());
            return ret;
        }
        // LOG_DEBUG(logger, "RX READ POINTER: %u", readPointer);
        
        // read data from the Rx buffer
        const bsb_e bsb = static_cast<bsb_e>(4*static_cast<uint8_t>(mID) + 0x03);
        const uint8_t controlPhase  = Converter::ControlPhase(bsb, am_e::READ);
        // LOG_DEBUG(logger, "length to read: %u", length);
        ret = mW5500.read(readPointer, controlPhase, length, data);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE FROM RX BUFFER: %s", ret.c_str());
            return ret;
        }

        readPointer += length;
        ret = mW5500.writeSRB(mID, srb_addr_e::RX_READ_POINTER, readPointer);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE RX READ POINTER: %s", ret.c_str());
            return ret;
        }
        
        // receive data stored in the Rx buffer
        ret = mW5500.writeSRB(mID, srb_addr_e::COMMAND, static_cast<uint8_t>(scr_e::RECEIVE));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RECEIVE DATA: %s", ret.c_str());
            return ret;
        }

        // check the result of sending
        waitCommandCleared();
        return ret;
    }
}}

#endif