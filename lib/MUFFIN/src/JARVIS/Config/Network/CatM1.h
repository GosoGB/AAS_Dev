/**
 * @file CatM1.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "Common/DataStructure/bitset.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs { namespace config {

    class CatM1 : public Base
    {
    public:
        CatM1() : Base(cfg_key_e::LTE_CatM1) {}
        virtual ~CatM1() override {}
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
        typedef enum class SetFlagEnum
            : uint8_t
        {
            MODEL    = 0,
            COUNTRY  = 1,
            TOP      = 2
        } set_flag_e;
        bitset<static_cast<uint8_t>(set_flag_e::TOP)> mSetFlags;
    private:
        md_e mModel;
        ctry_e mCountry;
    };


    extern CatM1* catM1;
}}}