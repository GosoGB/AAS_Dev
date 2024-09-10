/**
 * @file CatM1.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-09-05
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Logger/Logger.h"
#include "CatM1.h"



namespace muffin { namespace jarvis { namespace config {

    CatM1::CatM1()
        : Base("lte")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    CatM1::~CatM1()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    void CatM1::operator=(const CatM1& obj)
    {
        mModel   = obj.mModel;
        mCountry = obj.mCountry;
    }

    bool CatM1::operator==(const CatM1& obj) const
    {
        return mModel == obj.mModel && mCountry == obj.mCountry;
    }

    bool CatM1::operator!=(const CatM1& obj) const
    {
        return !(*this == obj);
    }

    Status CatM1::SetModel(const std::string& model)
    {
        assert(model == "LM5" || model == "LCM300");

        if (model == "LM5")
        {
            LOG_INFO(logger, "LM5 model is selected");
            mModel = model_e::LM5;
        }
        else
        {
            LOG_INFO(logger, "LCM300 model is selected");
            mModel = model_e::LCM300;
        }

        return Status(Status::Code::GOOD);
    }

    Status CatM1::SetCounty(const std::string& country)
    {
        assert(country == "KR" || country == "USA");

        if (country == "KR")
        {
            mCountry = country_e::KOREA;
        }
        else
        {
            mCountry = country_e::USA;
        }

        return Status(Status::Code::GOOD);
    }

    CatM1::model_e CatM1::GetModel() const
    {
        return mModel;
    }

    CatM1::country_e CatM1::GetCountry() const
    {
        return mCountry;
    }
}}}