/**
 * @file Variable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 변수형 데이터를 표현하는 Variable Node 클래스를 선언합니다.
 * 
 * @date 2024-09-26
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Base.h"



namespace muffin { namespace im {

    class Variable : public Base
    {
    public:
        Variable(const NodeID& nodeID, const qualified_name_t& browseName);
        virtual ~Variable();
    private:
        /* data */
    };
}}