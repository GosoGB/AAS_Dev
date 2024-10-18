/**
 * @file MetaDataValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 메타데이터가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-15
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
        std::pair<rsc_e, std::string> Inspect(const JsonObject json);
    public:
        std::pair<Status, prtcl_ver_e> RetrieveProtocolVersion() const;
        std::pair<Status, std::string> RetrieveRequestID() const;
        std::pair<Status, std::set<cfg_key_e>> RetrieveContainerKeys() const;
    private:
        rsc_e validateVersion(const JsonObject json);
        rsc_e validateRequestID(const JsonObject json);
        rsc_e validateContainer(const JsonObject json);
    private:
        /*Protocol Version*/
        static constexpr uint8_t SUPPORTED_VERSION_LENGTH = 1;
        static prtcl_ver_e SUPPORTED_VERSION[SUPPORTED_VERSION_LENGTH];
        prtcl_ver_e mVersion;
        rsc_e mVersionState;
    private:
        /*Request Identifier*/
        std::string mRequestID;
    private:
        /*Config Instance Container*/
        uint8_t mContainerLength;
        std::set<cfg_key_e> mContainerKeySet;
    private:
        bool mHasNoError = false;
        std::string mResponseDescription;
    };
}}