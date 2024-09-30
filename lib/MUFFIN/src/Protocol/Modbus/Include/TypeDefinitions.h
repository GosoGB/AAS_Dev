/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus와 관련된 데이터 타입들을 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Status.h"



namespace muffin { namespace modbus {


    typedef enum class ModbusAreaEnum
        : uint8_t
    {
        COIL              = 1,
        DISCRETE_INPUT    = 2,
        INPUT_REGISTER    = 3,
        HOLDING_REGISTER  = 4
    } area_e;
}}