/**
 * @file RetrieveServiceNicService.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정을 따라 서비스 네트워크를 반환하는 서비스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Network/INetwork.h"



namespace muffin {

    INetwork* RetrieveServiceNicService();
}