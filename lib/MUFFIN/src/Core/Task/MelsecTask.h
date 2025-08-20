/**
 * @file MelsecTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Melsec 프로토콜로 데이터를 수집하는 태스크를 선언합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Protocol/Melsec/Melsec.h"


namespace muffin {

    void StartMelsecTask();
    void StopMelsecTask();
    bool HasMelsecTask();

    extern std::vector<Melsec> MelsecVector;
}