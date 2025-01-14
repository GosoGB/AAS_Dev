/**
 * @file Constants.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 사용하는 상수를 정의합니다.
 * 
 * @date 2025-01-13
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <sys/_stdint.h>



namespace muffin {

    static constexpr uint8_t  MAX_RETRY_COUNT = 5;
    static constexpr uint16_t SECOND_IN_MILLIS = 1000;
    static constexpr uint16_t KILLOBYTE = 1024;
}