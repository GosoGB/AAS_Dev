/**
 * @file ModbusRTU.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-09-27
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "IM/Node/Node.h"



namespace muffin {

    class ModbusRTU
    {
    public:
        ModbusRTU();
        ~ModbusRTU();
    public:
        Status AddNodeReference(im::Node& const node);
    private:
        /* data */
    };
}