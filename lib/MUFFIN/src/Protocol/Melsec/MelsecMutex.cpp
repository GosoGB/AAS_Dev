/**
 * @file MelsecMutex.cpp
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec Mutex를 정의합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "MelsecMutex.h"



namespace muffin {

    SemaphoreHandle_t xSemaphoreMelsec = NULL;
}