/**
 * @file Base.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 모듈의 여러 설정 정보 클래스에 대한 추상화 클래스를 정의합니다.
 * 
 * @date 2024-09-02
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Base.h"



namespace muffin { namespace jvs { namespace config {

    Base::Base(const cfg_key_e category)
        : mCategory(category)
    {
    }
    
    Base::~Base()
    {
    }

    cfg_key_e Base::GetCategory() const
    {
        return mCategory;
    }
}}}