/**
 * @file Server.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-09-09
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#if defined(MT11)




#include "./Server.hpp"



namespace muffin { namespace w5500 {

    
    void Server::Begin(const uint16_t port)
    {
        Begin(port, true);
    }


    void Server::Begin(const uint16_t port, bool isReusable)
    {
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

        mSocket.reset(new Socket(*ethernet, sock_id_e::SOCKET_6, w5500::sock_prtcl_e::TCP));
        if (mSocket.get() == nullptr)
        {
            LOG_ERROR(logger, "SOCK ERROR: SOCKET CREATION FAILED");
            return;
        }

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

        mNoDelay = false;
        mListening = true;
        // mAcceptedSocket = -1;
    }


}}


#endif