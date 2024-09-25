/**
 * @file Variable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 변수형 데이터를 표현하는 Variable Node 클래스를 정의합니다.
 * 
 * @date 2024-09-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Variable.h"



namespace muffin { namespace im {

    Variable::Variable()
        : Base()
    {
        LOG_DEBUG(logger, "mNodeID: %s", mNodeID.GetID().String.Data);
    }
    
    Variable::~Variable()
    {
    }
}}