/**
 * @file MUFFIN.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크에 대한 공개 인터페이스를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Core/Core.h"
#include "MUFFIN.h"



void MUFFIN::Start()
{
    muffin::Core& core = muffin::Core::GetInstance();
    core.Init();
}