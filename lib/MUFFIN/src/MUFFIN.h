/**
 * @file MUFFIN.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에 대한 공개 인터페이스를 선언합니다.
 * @details 본 파일은 MUFFIN 프레임워크를 그대로 사용하는 경우 내부 구현의 변경으로
 *          인해 발생할 수 있는 문제를 방지하기 위한 공개 인터페이스를 제공합니다.
 *          왜냐하면 개발자가 내부 구현에 직접 접근하는 경우에는 프레임워크의 내부 
 *          구현의 변경은 문제가 없지만 프레임워크를 그대로 사용하는 경우에는 문제가
 *          될 수 있기 떄문입니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#if __cplusplus < 201103L && (!defined(_MSC_VER) || _MSC_VER < 1910)
    # error MUFFIN requires C++11 or newer. Configure your compiler for C++11.
#endif

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>



class MUFFIN
{
public:
    MUFFIN() {}
    virtual ~MUFFIN() {}
public:
    void Start();
};