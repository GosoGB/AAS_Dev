/**
 * @file Server.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../../Common/PSRAM.hpp"

namespace muffin { namespace aas {


    class Server
    {
    public:
        Server(Server const&) = delete;
        void operator=(Server const&) = delete;
        static Server* GetInstance();
    private:
        Server() = default;
        ~Server() noexcept = default;
    private:
        static Server* mInstance;
    
    public:
        void Init();
    };
}}