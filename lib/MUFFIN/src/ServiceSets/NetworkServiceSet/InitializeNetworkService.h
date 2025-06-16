/**
 * @file InitializeNetworkService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 초기화 함수를 선언합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"



namespace muffin {

    Status InitCatM1Service();
#if !defined(MODLINK_L)
    Status InitEthernetService();
#endif 
}