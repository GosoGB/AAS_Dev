/**
 * @file CyclicalPubTask.h
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



namespace muffin {

    void StartTaskCyclicalsMSG();
    void StopCyclicalsMSGTask();
}