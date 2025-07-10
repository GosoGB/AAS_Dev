/**
 * @file EthernetIpMutex.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief EthernetIP Mutex를 정의합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)

#pragma once


#include "freertos/semphr.h"


namespace muffin {

    extern SemaphoreHandle_t xSemaphoreEthernetIP;
    
}

#endif