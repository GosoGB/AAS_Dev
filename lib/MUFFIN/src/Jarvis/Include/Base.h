/**
 * @file Base.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 모듈의 여러 설정 정보 클래스에 대한 추상화 클래스를 선언합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class Base
    {
    public:
        explicit Base(const cfg_key_e category);
        virtual ~Base();
    public:
        cfg_key_e GetCategory() const;
        /**
         * @todo 모든 config 클래스가 공통으로 지원하는 print 함수를 만들면 좋겠습니다.
         */
        // virtual const char* ToString() const = 0;
    private:
        const cfg_key_e mCategory;
    };
}}}