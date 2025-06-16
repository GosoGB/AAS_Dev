/**
 * @file MelsecMutex.h
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec Mutex를 정의합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */



#pragma once


#include "freertos/semphr.h"


namespace muffin {

    extern SemaphoreHandle_t xSemaphoreMelsec;
    
}