#if defined(MB10) || defined(MT10) || defined(MT11)

/**
 * @file EthernetFactory.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Network/INetwork.h"



namespace muffin {

    extern std::string ntpServer;
    
    class EthernetFactory
    {
    public:
        static INetwork* Create();
    };
}


#endif