/**
 * @file CatM1.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "CatM1.h"
#include "Common/Assert.h"



namespace muffin { namespace jvs { namespace config {

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
        return (
            (mModel == obj.mModel)  &&
            (mCountry == obj.mCountry)
        );
    }

    bool CatM1::operator!=(const CatM1& obj) const
    {
        return !(*this == obj);
    }

    void CatM1::SetModel(const md_e model)
    {
        mModel = model;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::MODEL));
    }

    void CatM1::SetCounty(const ctry_e country)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::MODEL)) == true), "MODEL MUST BE SET BEFOREHAND");
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
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::COUNTRY));
    }

    std::pair<Status, md_e> CatM1::GetModel() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::MODEL)))
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
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::COUNTRY)))
        {
            return std::make_pair(Status(Status::Code::GOOD), mCountry);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mCountry);
        }
    }
}}}