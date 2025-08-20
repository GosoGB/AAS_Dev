/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus와 관련된 데이터 타입들을 선언합니다.
 * 
 * @date 2024-10-01
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <set>
#include <string>
#include <sys/_stdint.h>
#include <IPAddress.h>

#include "IM/Node/Include/NumericAddressRange.h"
#include "Common/Status.h"


namespace muffin { namespace modbus {


    typedef struct ModbusPolledDatumType  /* 32 bits */
    {
        uint16_t  Address;
        uint16_t  Value;
        bool IsOK;
    } datum_t;

    typedef struct ModbusTcpServerStruct  /* 32 bits */
    {
        IPAddress serverIP;
        uint16_t serverPort;
    } modbusTcpServer_t;
}}