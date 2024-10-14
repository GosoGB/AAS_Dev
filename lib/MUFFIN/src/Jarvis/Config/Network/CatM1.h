/**
 * @file CatM1.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class CatM1 : public Base
    {
    public:
        CatM1();
        virtual ~CatM1() override;
    public:
        CatM1& operator=(const CatM1& obj);
        bool operator==(const CatM1& obj) const;
        bool operator!=(const CatM1& obj) const;
    public:
        void SetModel(const md_e model);
        void SetCounty(const ctry_e country);
    public:
        std::pair<Status, md_e> GetModel() const;
        std::pair<Status, ctry_e> GetCountry() const;
    private:
        bool isModelSet   = false;
        bool isCountrySet = false;
    private:
        md_e mModel;
        ctry_e mCountry;
    };
}}}