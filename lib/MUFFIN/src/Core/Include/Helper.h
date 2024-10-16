/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Core 모듈 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <esp_system.h>
#include <string>

#include "Common/Status.h"



namespace muffin {

    void printResetReason(const esp_reset_reason_t reason);
    Status readMacAddressEthernet(std::string* mac);
    Status readMacAddressWiFiClient(std::string* mac);
    Status readMacAddressWiFiServer(std::string* mac);
}