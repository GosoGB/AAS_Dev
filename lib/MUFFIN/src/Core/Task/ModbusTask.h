/**
 * @file ModbusTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Modbus 프로토콜로 데이터를 수집하는 태스크를 선언합니다.
 * 
 * @date 2024-10-21
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once



namespace muffin {

    void StartModbusTask();
    void StopModbusTask();
    void SetPollingInterval(const uint16_t pollingInterval);
    bool HasModbusTask();
}