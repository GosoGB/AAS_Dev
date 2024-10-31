/**
 * @file NetworkTask.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 사용과 관련된 태스크를 선언합니다.
 * 
 * @date 2024-10-30
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Config/Network/CatM1.h"



namespace muffin {

    Status InitCatM1(jarvis::config::CatM1* cin);
    Status InitCatHTTP();
    Status ConnectToBroker();

    void StartCatM1Task();
}