/**
 * @file EthernetClient.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-19
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)



#include "Common/Assert.h"
#include "DNS.h"
#include "EthernetClient.h"



namespace muffin { namespace w5500 {

    EthernetClient* embededEthernetClient = nullptr;
    EthernetClient* link1EthernetClient   = nullptr;
    class EthernetClientRxBuffer
    {
    public:
        EthernetClientRxBuffer(Socket& socket, size_t size=1436)
            : mSize(size)
            , mBuffer(NULL)
            , mPos(0)
            , mFill(0)
            , mSocket(socket)
            , mHasFailed(false)
        {
        }
    
        ~EthernetClientRxBuffer()
        {
            free(mBuffer);
        }
    
        bool failed()
        {
            return mHasFailed;
        }
    
        int read(uint8_t* dst, size_t len)
        {
            if (!dst || !len || (mPos == mFill && !fillBuffer()))
            {
                return mHasFailed ? -1 : 0;
            }
    
            size_t a = mFill - mPos;
            if (len <= a || ((len - a) <= (mSize - mFill) && fillBuffer() >= (len - a)))
            {
                if (len == 1)
                {
                    *dst = mBuffer[mPos];
                }
                else
                {
                    memcpy(dst, mBuffer + mPos, len);
                }
                mPos += len;
                return len;
            }
    
            size_t left = len;
            size_t toRead = a;
            uint8_t* buf = dst;
            memcpy(buf, mBuffer + mPos, toRead);
            mPos += toRead;
            left -= toRead;
            buf += toRead;

            while (left)
            {
                if (!fillBuffer())
                {
                    return len - left;
                }
                
                a = mFill - mPos;
                toRead = (a > left)?left:a;
                memcpy(buf, mBuffer + mPos, toRead);
                mPos += toRead;
                left -= toRead;
                buf += toRead;
            }

            return len;
        }
    
        int peek()
        {
            if (mPos == mFill && !fillBuffer())
            {
                return -1;
            }

            return mBuffer[mPos];
        }
    
        size_t available()
        {
            return mFill - mPos + r_available();
        }
    
    private:
        size_t mSize;
        uint8_t* mBuffer;
        size_t mPos;
        size_t mFill;
        Socket& mSocket;
        bool mHasFailed;
    
        size_t r_available()
        {
            int count = mSocket.Available();
            if (count < 0)
            {
                mHasFailed = true;
                return 0;
            }
            
            return count;
        }
    
        size_t fillBuffer()
        {
            if (!mBuffer)
            {
                mBuffer = (uint8_t *)malloc(mSize);
                if (!mBuffer)
                {
                    log_e("Not enough memory to allocate buffer");
                    mHasFailed = true;
                    return 0;
                }
            }
            
            if (mFill && mPos == mFill)
            {
                mFill = 0;
                mPos = 0;
            }
    
            if (!mBuffer || mSize <= mFill || !r_available())
            {
                return 0;
            }
    
            size_t actualLength = 0;
            Status ret = mSocket.Receive(mSize - mFill, &actualLength, mBuffer + mFill);
            if (ret != Status::Code::GOOD)
            {
                mHasFailed = true;
                return 0;
            }
            
            mFill += actualLength;
            return actualLength;
        }
    };


    constexpr uint16_t DEFAULT_TIMEOUT_MS  =    3000;
    constexpr uint8_t  MAX_TRIAL_COUNT     =      10;
    constexpr uint32_t CLIENT_TIMEOUT_US   = 1000000;
    constexpr uint16_t CLIENT_BUFFER_SIZE  =    1024;


    EthernetClient::EthernetClient()
        : mIsConnected(false)
        , mTimeout(3000)
    {
    }


    EthernetClient::EthernetClient(W5500& interface, const sock_id_e idx)
        : mIsConnected(true)
        , mTimeout(3000)
    {
        mSocket.reset(new Socket(interface, idx, w5500::sock_prtcl_e::TCP));
        mRxBuffer.reset(new EthernetClientRxBuffer(*mSocket, 1436));
    }


    EthernetClient::~EthernetClient()
    {
        stop();
    }


    EthernetClient& EthernetClient::operator=(const EthernetClient &other)
    {
        stop();
        mSocket = other.mSocket;
        mRxBuffer = other.mRxBuffer;
        mIsConnected = other.mIsConnected;
        return *this;
    }

    
    void EthernetClient::stop()
    {
        mSocket->Close();
        mIsConnected = false;
        mRxBuffer = nullptr;
        
    }


    int EthernetClient::connect(IPAddress ip, uint16_t port)
    {
        return connect(ip, port, mTimeout);
    }


    int EthernetClient::connect(IPAddress ip, uint16_t port, int32_t timeout_ms)
    {
        mTimeout = timeout_ms;
        if (mSocket == nullptr)
        {
            LOG_WARNING(logger,"SOCKET IS NULL!!!!!!!!!!!!");
            return 0;
            
        }
        
        Status ret = mSocket->Open();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN SOCKET: %s", ret.c_str());
            return 0;
        }

        ret = mSocket->Connect(ip, port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT: %s", ret.c_str());
            return 0;
        }
        

        mRxBuffer.reset(new EthernetClientRxBuffer(*mSocket, 1436));
        mIsConnected = true;
        return 1;
    }


    int EthernetClient::connect(const char* host, uint16_t port)
    {
        return connect(host, port, mTimeout);
    }

    
    int EthernetClient::connect(const char* host, uint16_t port, int32_t timeout_ms)
    {
        Socket socket(mSocket->mW5500, sock_id_e::SOCKET_7, sock_prtcl_e::UDP);
        DNS dns(socket);
    #if !defined(DEBUG)
        dns.Init 할 때 MFM에서 설정 받거나 DHCP에서 받은
        DNS 서버 주소를 사용할 수 있도록 수정해야 함!
    #else
        Status ret = dns.Init(IPAddress(8, 8, 8, 8));
    #endif
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE DNS: %s", ret.c_str());
            return 0;
        }

        IPAddress resolvedIPv4;
        ret = dns.GetHostByName(host, &resolvedIPv4);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RESOLVE HOST: %s", ret.c_str());
            return 0;
        }

        return connect(resolvedIPv4, port, timeout_ms);
    }


    size_t EthernetClient::write(uint8_t data)
    {
        return write(&data, 1);
    }
    
    
    int EthernetClient::read()
    {
        uint8_t data = 0;
        int res = read(&data, 1);
        if (res < 0)
        {
            return res;
        }

        if (res == 0)
        {//  No data available.
            return -1;
        }

        return data;
    }


    size_t EthernetClient::write(const uint8_t* buf, size_t size)
    {
        int res = 0;
        int retry = MAX_TRIAL_COUNT;
        size_t totalBytesSent = 0;
        size_t bytesRemaining = size;
        LOG_DEBUG(logger, "bytesRemaining: %u", bytesRemaining);

        if (mIsConnected == false)
        {
            return 0;
        }
        
        Status ret(Status::Code::UNCERTAIN);
        while (retry)
        {
            retry--;
            ret = mSocket->Send(bytesRemaining, buf);
            LOG_DEBUG(logger, "mSocket->Send: ret: %s", ret.c_str());
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SEND DATA: %s", ret.c_str());
                if (ret == Status::Code::BAD_RESOURCE_UNAVAILABLE)
                {
                    continue;
                }
                else
                {
                    stop();
                    res = 0;
                    retry = 0;
                }
            }
            res = size;
            totalBytesSent = res;
            LOG_DEBUG(logger, "Sent data: %u bytes(%u bytes)", res, size);
            
            if (totalBytesSent >= size)
            {// completed successfully
                retry = 0;
            }
            else
            {
                buf += res;
                bytesRemaining -= res;
                retry = MAX_TRIAL_COUNT;
            }
        }

        return totalBytesSent;
    }


    size_t EthernetClient::write_P(PGM_P buf, size_t size)
    {
        return write(reinterpret_cast<const uint8_t*>(buf), size);
    }


    size_t EthernetClient::write(Stream &stream)
    {
        uint8_t* buf = (uint8_t*)malloc(1360);
        if (!buf)
        {
            return 0;
        }

        size_t toRead = 0, toWrite = 0, written = 0;
        size_t available = stream.available();
        while (available)
        {
            toRead = (available > 1360)?1360:available;
            toWrite = stream.readBytes(buf, toRead);
            written += write(buf, toWrite);
            available = stream.available();
        }

        free(buf);
        return written;
    }


    int EthernetClient::read(uint8_t* buf, size_t size)
    {
        int res = -1;
        if (mRxBuffer)
        {
            res = mRxBuffer->read(buf, size);
            if (mRxBuffer->failed())
            {
                LOG_ERROR(logger, "FAILED TO READ: fd %d, errno: %d, \"%s\"", 
                    static_cast<uint8_t>(mSocket->GetSocketID()), errno, strerror(errno));
                stop();
            }
        }

        return res;
    }


    int EthernetClient::peek()
    {
        int res = -1;
        if (mRxBuffer)
        {
            res = mRxBuffer->peek();
            if (mRxBuffer->failed())
            {
                LOG_ERROR(logger, "FAILED ON SOCKET %d, errno: %d, \"%s\"", 
                    static_cast<uint8_t>(mSocket->GetSocketID()), errno, strerror(errno));
                stop();
            }
        }
        return res;
    }


    int EthernetClient::available()
    {
        if (!mRxBuffer)
        {
            return 0;
        }

        int res = mRxBuffer->available();
        if (mRxBuffer->failed())
        {
            LOG_ERROR(logger, "FAILED ON SOCKET %d, errno: %d, \"%s\"", 
                static_cast<uint8_t>(mSocket->GetSocketID()), errno, strerror(errno));
            stop();
        }
        return res;
    }


    void EthernetClient::flush()
    {
        size_t remained = available();
        if (remained == 0)
        {
            return;
        }

        size_t toRead = 0;
        size_t actualLength = 0;
        uint8_t* buf = (uint8_t*)malloc(CLIENT_BUFFER_SIZE);

        if (!buf)
        {
            return;//memory error
        }
        
        while (remained)
        {
            toRead = (remained > CLIENT_BUFFER_SIZE) ? CLIENT_BUFFER_SIZE : remained;

            Status ret = mSocket->Receive(toRead, &actualLength, buf);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FLUSH: fd %d, errno: %d, \"%s\"", 
                    static_cast<uint8_t>(mSocket->GetSocketID()), errno, strerror(errno));
                stop();
                break;
            }
            
            remained -= actualLength;
        }

        free(buf);
    }


    uint8_t EthernetClient::connected()
    {
        if (mIsConnected == false)
        {
            return 0;
        }
        
        sir_t interrupt;
        Status ret = mSocket->GetInterrupt(&interrupt);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO GET INTERRUPT: %s", ret.c_str());
            return 1;
        }

        if (interrupt.DISCONNECTED == true)
        {
            mIsConnected = false;
            return 0;
        }

        ssr_e status = mSocket->GetStatus();
        if (status != ssr_e::ESTABLISHED)
        {
            mIsConnected = false;
            
            return 0;
        }

        mIsConnected = true;
        return 1;
    }
}}

#endif