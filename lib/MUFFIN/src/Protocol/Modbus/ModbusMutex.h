/**
 * @file ModbusMutex.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */



#pragma once


#include "freertos/semphr.h"


namespace muffin {

    extern SemaphoreHandle_t xSemaphoreModbusRTU;
    
}