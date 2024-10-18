/**
 * @file CatM1.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "CatM1.h"



namespace muffin { namespace jarvis { namespace config {

    CatM1::CatM1()
        : Base(cfg_key_e::LTE_CatM1)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    CatM1::~CatM1()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    CatM1& CatM1::operator=(const CatM1& obj)
    {
        if (this != &obj)
        {
            mModel   = obj.mModel;
            mCountry = obj.mCountry;
        }
        
        return *this;
    }

    bool CatM1::operator==(const CatM1& obj) const
    {
        return mModel == obj.mModel && mCountry == obj.mCountry;
    }

    bool CatM1::operator!=(const CatM1& obj) const
    {
        return !(*this == obj);
    }

    void CatM1::SetModel(const md_e model)
    {
        mModel = model;
        isModelSet = true;
    }

    void CatM1::SetCounty(const ctry_e country)
    {
        ASSERT((isModelSet == true), "MODEL MUST BE SET BEFOREHAND");
        ASSERT(
            (
                [&]()
                {
                    if (mModel == md_e::LM5 && country == ctry_e::USA)
                    {
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }()
            ), "LM5 MODEL CAN ONLY OPERATE IN KR"
        );

        mCountry = country;
        isCountrySet = true;
    }

    std::pair<Status, md_e> CatM1::GetModel() const
    {
        if (isModelSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mModel);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mModel);
        }
    }

    std::pair<Status, ctry_e> CatM1::GetCountry() const
    {
        if (isCountrySet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mCountry);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mCountry);
        }
    }
}}}