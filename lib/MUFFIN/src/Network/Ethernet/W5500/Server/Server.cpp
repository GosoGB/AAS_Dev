/**
 * @file Server.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-09-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)




#include "./Parser.hpp"
#include "./Server.hpp"

static const char AUTHORIZATION_HEADER[] = "Authorization";
static const char qop_auth[] PROGMEM = "qop=auth";
static const char qop_auth_quoted[] PROGMEM = "qop=\"auth\"";
static const char WWW_Authenticate[] = "WWW-Authenticate";
static const char Content_Length[] = "Content-Length";

static const uint16_t HTTP_MAX_DATA_WAIT   = 5000; //ms to wait for the client to send the request
static const uint16_t HTTP_MAX_POST_WAIT   = 5000; //ms to wait for POST data to arrive
static const uint16_t HTTP_MAX_SEND_WAIT   = 5000; //ms to wait for data chunk to be ACKed
static const uint16_t HTTP_MAX_CLOSE_WAIT  = 2000; //ms to wait for the client to close the connection

static const size_t CONTENT_LENGTH_UNKNOWN = (size_t) -1;
static const size_t CONTENT_LENGTH_NOT_SET = (size_t) -2;



namespace muffin { namespace w5500 {

    
    void Server::Begin(const uint16_t port)
    {
        Begin(port, true);
    }


    void Server::Begin(const uint16_t port, bool isReusable)
    {
        Close();

        if (mListening == true)
        {
            LOG_VERBOSE(logger, "Server has already begun. Nothing to do");
            return;
        }

        if (port != 0)
        {
            mPort = port;
        }
        ASSERT((port != 0), "CONFIG ERROR: PORT CANNOT BE 0");

        mSocket.reset(new Socket(*ethernet, sock_id_e::SOCKET_4, w5500::sock_prtcl_e::TCP));

        /**
         * @todo 여기 살려야 할지 확인 후 처리해야 함
         */
        mClient.reset(new EthernetClient(*ethernet, sock_id_e::SOCKET_3));
        if (mClient.get() == nullptr)
        {
            LOG_ERROR(logger, "CLIENT ERROR: CONSTRUCTION FAILED");
            return;
        }
        mClient->peek();
        //--------------------------------------------------------------------

        Status ret = mSocket->Open(port);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "SOCK ERROR: FAILED TO OPEN: '%s'", ret.c_str());
            return;
        }

        ret = mSocket->Listen();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "SOCK ERROR: FAILED TO LISTEN: '%s'", ret.c_str());
            return;
        }

        mListening = true;

        // while (true)
        // {
        //     const ssr_e status = mSocket->GetStatus();
        //     LOG_DEBUG(logger, "%s", Converter::ToString(status));
        //     if (status == ssr_e::CLOSE_WAIT)
        //     {
        //         mSocket->Disconnect();
        //     }
        //     else if (status == ssr_e::CLOSED)
        //     {
        //         mSocket->Open(port);
        //     }
        //     else if (status == ssr_e::INIT_TCP)
        //     {
        //         mSocket->Listen();
        //     }
            
        //     delay(1000);
        // }
    }


    void Server::collectHeaders(const char* headerKeys[], const size_t count)
    {
        mHeaderKeyCount = count + 1;
        if (mCurrentHeaders.size() > 0)
        {
            mCurrentHeaders.clear();
        }

        mCurrentHeaders.reserve(mHeaderKeyCount);
        mCurrentHeaders.shrink_to_fit();

        req_arg_t arg;
        arg.Key = AUTHORIZATION_HEADER;
        mCurrentHeaders.emplace_back(arg);

        for (int i = 1; i < mHeaderKeyCount; i++)
        {
            req_arg_t arg;
            arg.Key = headerKeys[i-1];
            mCurrentHeaders.emplace_back(arg);
        }
    }


    void Server::Close()
    {
        if (mSocket == nullptr)
        {
            return;
        }
        
        Status ret = mSocket->Close();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "SOCK ERROR: CLOSE FAILED '%s'", ret.c_str());
            return;
        }
        mListening = false;
        mHasClient = false;
        
        if (mHeaderKeyCount != 0)
        {
            collectHeaders(0, 0);
        }
    }


    void Server::Stop()
    {
        Close();
    }


    bool Server::HasClient()
    {
        ASSERT((mSocket != nullptr), "SOCKET CANNOT BE NULL");

        const ssr_e status = mSocket->GetStatus();

        switch (status)
        {
        case ssr_e::ESTABLISHED:
            mHasClient = true;
            break;
        default:
            mHasClient = false;
            break;
        }

        return mHasClient;
    }


    void Server::handleClient()
    {
        ASSERT((mSocket != nullptr), "SOCKET CANNOT BE NULL");

        const bool hasClient = HasClient();

        if (mClientStatus == __internal::client_status_e::NO_CLIENT)
        {
            if (hasClient == false)
            {
                return;
            }
            else
            {
                mClientStatus = __internal::client_status_e::WAIT_READ;
                mClientTimer  = millis();

                const IPAddress remoteIP = mSocket->GetRemoteIP();
                LOG_DEBUG(logger, "New client with IP=%s", remoteIP.toString().c_str());
            }
        }
        else
        {
            if (hasClient == false)
            {
                mClientStatus = __internal::client_status_e::NO_CLIENT;
                LOG_INFO(logger, "Client disconnected");
                return;
            }
        }
        
        bool keepCurrentClient = false;
        bool callYield = false;

        if (mSocket->Available() > 0)
        {
            ParseRequest(*mSocket.get());
            keepCurrentClient = true;
            callYield = true;
            // if (_parseRequest(_currentClient))
            // {
            //     mContentLength = CONTENT_LENGTH_NOT_SET;
            //     _handleRequest();
            // }
        }
        else
        {
            if (uint32_t(millis() - mClientTimer) <= HTTP_MAX_DATA_WAIT)
            {
                keepCurrentClient = true;
            }
            callYield = true;
        }

        if (keepCurrentClient == false)
        {
            mClientStatus = __internal::client_status_e::NO_CLIENT;
        }

        if (callYield == true)
        {
            yield();
        }
    }

    uint16_t Server::Available()
    {
        ASSERT((mSocket != nullptr), "SOCKET CANNOT BE NULL");
        return mSocket->Available();
    }
}}


#endif