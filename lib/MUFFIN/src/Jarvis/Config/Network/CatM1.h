/**
 * @file CatM1.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-09-05
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Config/Base.h"



namespace muffin { namespace jarvis { namespace config {

    class CatM1 : public Base
    {
    public:
        CatM1();
        virtual ~CatM1() override;
    public:
        void operator=(const CatM1& obj);
        bool operator==(const CatM1& obj) const;
        bool operator!=(const CatM1& obj) const;
    public:
        Status SetModel(const std::string& model);
        Status SetCounty(const std::string& country);
    public:
        typedef enum class CountryEnum
        {
            KOREA,
            USA,
            JAPAN,
            VIETNAM,
            TAIWAN
        } country_e;
        typedef enum class ModelEnum
        {
            LM5,
            LCM300
        } model_e;
    public:
        model_e GetModel() const;
        country_e GetCountry() const;
    private:
        model_e mModel;
        country_e mCountry;
    };
}}}