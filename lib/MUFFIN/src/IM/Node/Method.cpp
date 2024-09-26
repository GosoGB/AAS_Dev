/**
 * @file Method.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 원격제어에 대한 정보를 표현하는 Method Node 클래스를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Method.h"



namespace muffin { namespace im {

    Method::Method()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Method::~Method()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }
    
}}