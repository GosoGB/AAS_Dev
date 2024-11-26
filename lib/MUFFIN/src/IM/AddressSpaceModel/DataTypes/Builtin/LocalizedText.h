/**
 * @file LocalizedText.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief OPC UA 표준의 LocalizedText DataType을 정의합니다.
 * 
 * @date 2024-10-25
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>

#include "Common/Logger/Logger.h"



namespace muffin { namespace im {

    static constexpr const char* LocaleId[2] = { "en-US", "ko-KR" };

    typedef enum class LocaleIdEnum
        : uint8_t
    {
        EN_US  = 0,
        KO_KR  = 1
    } locale_id_e;


    class LocalizedText
    {
    public:
        LocalizedText()
            : mLocaleId(locale_id_e::EN_US)
            , mText("")
        {
        }

        LocalizedText(const locale_id_e localeId, const std::string& text)
            : mLocaleId(localeId)
            , mText(text)
        {
        }

        LocalizedText(const LocalizedText& obj)
            : mLocaleId(obj.mLocaleId)
            , mText(obj.mText)
        {
        }
        
        virtual ~LocalizedText()
        {
        }

        const char* GetLocaleId() const
        {
            return LocaleId[static_cast<uint8_t>(mLocaleId)];
        }

        std::string const GetText() const
        {
            return mText;
        }

    private:
        const locale_id_e mLocaleId;
        const std::string mText;
    };
}}