/**
 * @file Base.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델의 기본 단위인 Base Node 클래스를 정의합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <Arduino.h>
#include "Base.h"



namespace muffin { namespace im { 

    Base::Base(const NodeID& nodeID)
        : mNodeID(std::move(nodeID))
        , mClassType(class_type_e::UNSPECIFIED)
    {
        if (mNodeID.GetIdentifierType() == node_id_type_e::NUMERIC)
        {
            Serial.printf("%u \n", mNodeID.GetID().Numeric);
        }
        else
        {
            Serial.printf("%s \n", mNodeID.GetID().String.Data);
        }
    }
    
    Base::~Base()
    {
    }
}}