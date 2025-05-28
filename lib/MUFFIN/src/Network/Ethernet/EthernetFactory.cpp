#if defined(MB10) || defined(MT10) || defined(MT11)

/**
 * @file EthernetFactory.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "EthernetFactory.h"
#include "LAN8720/LAN8720.h"
#include "W5500/W5500.h"



namespace muffin {


    INetwork* EthernetFactory::Create()
    {
        if (ethernet == nullptr)
        {
            #if defined(MT10) || defined(MB10)
                ethernet = new LAN8720();
            #else
                ethernet = new W5500(w5500::if_e::EMBEDDED);
            #endif
        }

        return static_cast<INetwork*>(ethernet);
    }
}

#endif