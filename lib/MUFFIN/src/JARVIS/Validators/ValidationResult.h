/**
 * @file ValidationResult.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보에 대한 유효성 검사 결과를 표현하기 위한 클래스를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <vector>

#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class ValidationResult
    {
    public:
        ValidationResult();
        virtual ~ValidationResult();
    public:
        void SetRSC(const rsc_e rsc);
        void SetDescription(const std::string& desc);
        void EmplaceKeyWithNG(const cfg_key_e key);
    public:
        rsc_e GetRSC() const;
        uint8_t GetEmplacedKeyNum() const;
        std::string GetDescription() const;
        std::vector<cfg_key_e> RetrieveKeyWithNG() const;
    private:
        rsc_e mRSC;
        std::string mDescription;
        std::vector<cfg_key_e> mVectorConfigKeys;
    };
}}