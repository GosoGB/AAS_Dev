/**
 * @file ValidationResult.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보에 대한 유효성 검사 결과를 표현하기 위한 클래스를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "ValidationResult.h"



namespace muffin { namespace jvs {
    
    ValidationResult::ValidationResult()
    {
    }
    
    ValidationResult::~ValidationResult()
    {
    }
    
    void ValidationResult::SetRSC(const rsc_e rsc)
    {
        mRSC = rsc;
    }

    void ValidationResult::SetDescription(const std::string& desc)
    {
        mDescription = desc;
    }

    void ValidationResult::EmplaceKeyWithNG(const cfg_key_e key)
    {
        try
        {
            mVectorConfigKeys.emplace_back(key);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG KEY");
        }
    }

    rsc_e ValidationResult::GetRSC() const
    {
        return mRSC;
    }
    
    uint8_t ValidationResult::GetEmplacedKeyNum() const
    {
        return mVectorConfigKeys.size();
    }

    std::string ValidationResult::GetDescription() const
    {
        return mDescription;
    }

    std::vector<cfg_key_e> ValidationResult::RetrieveKeyWithNG() const
    {
        return mVectorConfigKeys;
    }
}}