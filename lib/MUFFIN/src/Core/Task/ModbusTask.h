/**
 * @file ModbusTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Modbus 프로토콜로 데이터를 수집하는 태스크를 선언합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Protocol/Modbus/Include/TypeDefinitions.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusRTU.h"


namespace muffin {

    void StartModbusRtuTask();
    void StopModbusRtuTask();
    bool HasModbusRtuTask();

    void StartModbusTcpTask();
    void StopModbusTcpTask();
    bool HasModbusTcpTask();

    extern std::vector<ModbusTCP> ModbusTcpVector;
    extern std::vector<ModbusTCP> ModbusTcpVectorDynamic;
    extern std::vector<ModbusRTU> ModbusRtuVector;
}