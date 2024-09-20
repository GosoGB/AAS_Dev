/**
 * @file Base.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 모듈의 여러 설정 정보 클래스에 대한 추상화 클래스를 선언합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @todo 생성자 매개변수를 std::string에서 enum class로 변경해야 합니다.
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
    private:
        const cfg_key_e mCategory;
    };
}}}