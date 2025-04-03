/**
 * @file MelsecMutex.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-10-22
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "MelsecMutex.h"



namespace muffin {

    SemaphoreHandle_t xSemaphoreMelsec = NULL;
}