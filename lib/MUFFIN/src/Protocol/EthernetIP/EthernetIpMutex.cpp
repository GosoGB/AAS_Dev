/**
 * @file EthernetIpMutex.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief EthernetIP Mutex를 정의합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */



#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"

#include "EthernetIpMutex.h"



namespace muffin {

    SemaphoreHandle_t xSemaphoreEthernetIP = NULL;
}