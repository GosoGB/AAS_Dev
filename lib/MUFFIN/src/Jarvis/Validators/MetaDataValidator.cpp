/**
 * @file MetaDataValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 메타데이터가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex> 

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Include/Helper.h"
#include "MetaDataValidator.h"



namespace muffin { namespace jarvis {

    MetaDataValidator::MetaDataValidator()
        : mIsMetaDataValid(false)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    MetaDataValidator::~MetaDataValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status MetaDataValidator::Inspect(const JsonObject json)
    {
        Status ret = validateVersion(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID VERSION: %s", ret.c_str());
            return ret;
        }

        ret = validateRequestID(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID REQUEST ID: %s", ret.c_str());
            return ret;
        }

        ret = validateContainer(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID CONTAINER: %s", ret.c_str());
            return ret;
        }

        LOG_INFO(logger, "Valid JARVIS metadata");
        mIsMetaDataValid = true;
        return ret;
    }

    Status MetaDataValidator::validateVersion(const JsonObject& json)
    {
        if (json.containsKey("ver") == false)
        {
            LOG_ERROR(logger, "VERSION NOT FOUND");
            return Status(Status::Code::BAD_NOT_FOUND);
        }
        
        std::smatch matches;
        const std::regex pattern(R"(^v([0-9]+)$)");
        const std::string& strVersion = json["ver"].as<std::string>();
        const bool hasMatch = std::regex_match(strVersion, matches, pattern);
        
        if (hasMatch == false)
        {
            LOG_ERROR(logger, "INVALID VERSION FORMAT");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        
        const uint8_t intVersion = ConvertToUInt8(matches[1].str().c_str());
        switch (intVersion)
        {
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_1):
            mContainerLength = 12;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_2):
            mContainerLength = UINT8_MAX;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_3):
            mContainerLength = UINT8_MAX;
            goto DEFINED_VERSION_FOUND;
        default:
            goto UNSUPPORTED_VERSION;
        }
    
    DEFINED_VERSION_FOUND:
        mVersion = static_cast<prtcl_ver_e>(intVersion);
        for (uint8_t idx = 0; idx < SUPPORTED_VERSION_LENGTH; ++idx)
        {
            if (SUPPORTED_VERSION[idx] == mVersion)
            {
                mVersion = static_cast<prtcl_ver_e>(intVersion);
                return Status(Status::Code::GOOD);
            }
        }
    
    UNSUPPORTED_VERSION:
        ASSERT(false, "UNDEFINED OR UNSUPPORTED VERSION: %u", intVersion);
        LOG_ERROR(logger,"UNDEFINED OR UNSUPPORTED VERSION: %u", intVersion);
        return Status(Status::Code::BAD_PROTOCOL_VERSION_UNSUPPORTED);
    }

    Status MetaDataValidator::validateRequestID(const JsonObject& json)
    {
        mRequestID.clear();

        if (json.containsKey("rqi") == false)
        {
            LOG_VERBOSE(logger, "No request identifier");
            return Status(Status::Code::GOOD);
        }

        if (json["rqi"].isNull() == false)
        {
            mRequestID = json["rqi"].as<std::string>();
            LOG_VERBOSE(logger, "Request identifier: %s", mRequestID.c_str());
            return Status(Status::Code::GOOD);
        }

        LOG_ERROR(logger, "REQUEST ID CANNOT BE NULL");
        return Status(Status::Code::BAD_NOT_FOUND);
    }

    Status MetaDataValidator::validateContainer(const JsonObject& json)
    {
        mContainerKeySet.clear();

        if (json.containsKey("cnt") == false)
        {
            LOG_ERROR(logger, "CONTAINER NOT FOUND");
            return Status(Status::Code::BAD_NO_ENTRY_EXISTS);
        }

        if (json["cnt"].is<JsonObject>() == false)
        {
            LOG_ERROR(logger, "CONTAINER MUST BE A JSON OBJECT");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        
        JsonObject container = json["cnt"];
        if (container.size() != mContainerLength)
        {
            LOG_ERROR(logger, "INVALID CONTAINER LENGTH FOR THE PROTOCOL VERSION: %u", container.size());
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }

        Status ret(Status::Code::UNCERTAIN);
        for (auto config : container)
        {
            if (config.value().is<JsonArray>() == false)
            {
                LOG_ERROR(logger, "CONFIG INSTANCE MUST BE A JSON ARRAY: %s", config.key().c_str());
                ret = Status::Code::BAD_ENCODING_ERROR;
                goto INVALID_CONTAINER;
            }
            
            const auto retKey = ConvertToConfigKey(mVersion, config.key().c_str());
            if (retKey.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "CONTAINER HAS INVALID CONFIG KEY: %s", retKey.first.c_str());
                ret = retKey.first;
                goto INVALID_CONTAINER;
            }

            if (config.value().as<JsonArray>().size() == 0)
            {
                LOG_VERBOSE(logger, "CONFIG HAS AN EMPTY ARRAY: %s", config.key().c_str());
                continue;
            }
            
            const auto retEmplace = mContainerKeySet.emplace(retKey.second);
            if (retEmplace.second == false)
            {
                LOG_ERROR(logger, "CONFIG KEY CANNOT BE DUPLICATED: %s", config.key().c_str());
                ret = Status::Code::BAD_ENTRY_EXISTS;
                goto INVALID_CONTAINER;
            }
        }
        
        return Status(Status::Code::GOOD);

    INVALID_CONTAINER:
        mContainerKeySet.clear();
        return ret;
    }

    prtcl_ver_e MetaDataValidator::RetrieveProtocolVersion() const
    {
        ASSERT((mIsMetaDataValid == true), "CANNOT RETRIEVE INVALID PROTOCOL VERSION");

        return mVersion;
    }

    const std::string& MetaDataValidator::RetrieveRequestID() const
    {
        ASSERT((mIsMetaDataValid == true), "CANNOT RETRIEVE INVALID REQUEST ID");

        return mRequestID;
    }

    const std::set<cfg_key_e>& MetaDataValidator::RetrieveContainerKeys() const
    {
        ASSERT((mIsMetaDataValid == true), "CANNOT RETRIEVE INVALID CONTAINER KEYS");

        return mContainerKeySet;
    }
}}