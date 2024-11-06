/**
 * @file Assert.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에서 사용하는 ASSERT 매크로 함수를 정의합니다.
 * 
 * @date 2024-09-11
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <cstdlib>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <iostream>



#if defined(DEBUG)
    #define ASSERT(condition, format, ...) \
        do { \
            if (condition == false) \
            { \
                char buffer[128]; \
                std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
                std::cerr << "\n\n\033[31m" \
                          << "ASSERTION FAILED: (" #condition ") \n" \
                          << "FILE: " << __FILE__ << "\nLINE: " << __LINE__ << " \n" \
                          << "LOG: " << buffer << "\n\n" << std::endl; \
                vTaskDelay(UINT32_MAX / portTICK_PERIOD_MS); \
                std::abort(); \
            } \
        } while (false)
#else
    #define ASSERT(condition, format, ...) ((void)0)
#endif