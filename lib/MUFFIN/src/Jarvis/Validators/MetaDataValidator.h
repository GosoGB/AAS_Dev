/**
 * @file MetaDataValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 메타데이터가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <set>

#include "Common/Status.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class MetaDataValidator
    {
    public:
        MetaDataValidator();
        virtual ~MetaDataValidator();
    public:
        Status Inspect(const JsonObject json);
        prtcl_ver_e RetrieveProtocolVersion() const;
        const std::string& RetrieveRequestID() const;
        const std::set<cfg_key_e>& RetrieveContainerKeys() const;
    private:
        Status validateVersion(const JsonObject& json);
        Status validateRequestID(const JsonObject& json);
        Status validateContainer(const JsonObject& json);
    private:
        /*Protocol Version*/
        static constexpr uint8_t SUPPORTED_VERSION_LENGTH = 1;
        static constexpr prtcl_ver_e SUPPORTED_VERSION[SUPPORTED_VERSION_LENGTH] =
        {
            prtcl_ver_e::VERSEOIN_1
        };
        prtcl_ver_e mVersion;
    private:
        /*Request Identifier*/
        std::string mRequestID;
    private:
        /*Config Instance Container*/
        uint8_t mContainerLength;
        std::set<cfg_key_e> mContainerKeySet;
    private:
        bool mIsMetaDataValid;
    };
}}