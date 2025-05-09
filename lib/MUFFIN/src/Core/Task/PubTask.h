/**
 * @file PubTask.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <sys/_stdint.h>
#include "Common/DataStructure/bitset.h"



namespace muffin {

    void StartTaskMSG();
    void StopMSGTask();
    typedef enum class SetFlagEnum : uint8_t
    {
        MODBUS_RTU_TASK   = 0,
        MODBUS_TCP_TASK   = 1,
        MELSEC_TASK       = 2,
    } set_task_flag_e;
    
    extern bitset<static_cast<uint8_t>(3)> s_DaqTaskEnableFlag;
    extern bitset<static_cast<uint8_t>(3)> s_DaqTaskSetFlag;
}