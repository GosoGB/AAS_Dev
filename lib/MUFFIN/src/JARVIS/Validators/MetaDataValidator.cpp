/**
 * @file MetaDataValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 메타데이터가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex> 

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "MetaDataValidator.h"



namespace muffin { namespace jvs {

    MetaDataValidator::MetaDataValidator()
        : mHasNoError(false)
    {
    }
    
    MetaDataValidator::~MetaDataValidator()
    {
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD
     *      @li BAD_INVALID_VERSION
     *      @li BAD_INVALID_VERSION_CONFIG_INSTANCES
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCES
     *      @li UNCERTAIN_VERSION_CONFIG_INSTANCES
     */
    std::pair<rsc_e, std::string> MetaDataValidator::Inspect(const JsonObject json)
    {
        std::pair<bool, rsc_e> pairUncertainRSC = {false, rsc_e::UNCERTAIN};

        rsc_e rsc = validateVersion(json);
        if (rsc == rsc_e::BAD || rsc == rsc_e::BAD_INVALID_VERSION)
        {
            LOG_ERROR(logger, "%s", mResponseDescription.c_str());
            return std::make_pair(rsc, mResponseDescription);
        }
        else if (rsc == rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            pairUncertainRSC.first = true;
            pairUncertainRSC.second = rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES;
        }

        rsc = validateRequestID(json);
        if (rsc != rsc_e::GOOD)
        {
            LOG_ERROR(logger, "%s", mResponseDescription.c_str());
            return std::make_pair(rsc, mResponseDescription);
        }

        rsc = validateContainer(json);
        if (rsc != rsc_e::GOOD && rsc != rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            LOG_ERROR(logger, "%s", mResponseDescription.c_str());
            return std::make_pair(rsc, mResponseDescription);
        }
        else if (rsc == rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
        {
            pairUncertainRSC.first = true;
            pairUncertainRSC.second = rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES;
        }
        mHasNoError = true;

        if (pairUncertainRSC.first == false)
        {
            LOG_INFO(logger, "JARVIS metadata: %s", mResponseDescription.c_str());
            return std::make_pair(rsc_e::GOOD, mResponseDescription);
        }
        else
        {
            LOG_WARNING(logger, "UNCERTAIN JARVIS METADATA: %X", static_cast<uint16_t>(pairUncertainRSC.second));
            return std::make_pair(pairUncertainRSC.second, mResponseDescription);
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD
     *      @li BAD_INVALID_VERSION
     *      @li UNCERTAIN_VERSION_CONFIG_INSTANCES
     */
    rsc_e MetaDataValidator::validateVersion(const JsonObject json)
    {
        if (json.containsKey("ver") == false)
        {
            mResponseDescription = "INVALID VERSION: KEY NOT FOUND";
            return rsc_e::BAD;
        }
   
        std::smatch matches;
        const std::regex pattern(R"(^v([0-9]+)$)");
        const std::string& strVersion = json["ver"].as<std::string>();
        const bool hasMatch = std::regex_match(strVersion, matches, pattern);
        
        if (hasMatch == false)
        {
            mResponseDescription = "INVALID VERSION: UNDEFINED FORMAT";
            return rsc_e::BAD;
        }
        const uint8_t intVersion = Convert.ToUInt8(matches[1].str().c_str());
        switch (intVersion)
        {
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_1):
            mContainerLength = 12;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_2):
            mContainerLength = 12;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_3):
            mContainerLength = 12;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_4):
            mContainerLength = 13;
            goto DEFINED_VERSION_FOUND;
        case static_cast<uint8_t>(prtcl_ver_e::VERSEOIN_5):
            mContainerLength = 14;
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
                mResponseDescription = "GOOD";
                return rsc_e::GOOD;
            }
        }
        goto INVALID_VERSION;
    
    INVALID_VERSION:
        mResponseDescription = "INVALID VERSION: " + std::to_string(intVersion);
        mVersionState = rsc_e::BAD_INVALID_VERSION;
        return mVersionState;

    UNSUPPORTED_VERSION:
        mResponseDescription = "UNSUPPORTED VERSION: " + std::to_string(intVersion);
        mVersionState = rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES;
        return mVersionState;
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD
     */
    rsc_e MetaDataValidator::validateRequestID(const JsonObject json)
    {
        mRequestID.clear();

        if (json.containsKey("rqi") == false)
        {
            mResponseDescription = "GOOD";
            return rsc_e::GOOD;
        }

        if (json["rqi"].isNull() == false)
        {
            if (json["rqi"].is<std::string>() == true)
            {
                mRequestID = json["rqi"].as<std::string>();
                mResponseDescription = "GOOD: " + mRequestID;
                return rsc_e::GOOD;
            }
            else
            {
                mResponseDescription = "INVALID REQUEST ID: MUST BE STRING";
                return rsc_e::BAD;
            }
        }

        mResponseDescription = "INVALID REQUEST ID: NOT NULLABLE WHEN KEY EXISTS";
        return rsc_e::BAD;
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD
     *      @li BAD_INVALID_VERSION_CONFIG_INSTANCES
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCES
     *      @li UNCERTAIN_VERSION_CONFIG_INSTANCES
     */
    rsc_e MetaDataValidator::validateContainer(const JsonObject json)
    {
        mContainerKeySet.clear();

        if (json.containsKey("cnt") == false)
        {
            mResponseDescription = "INVALID CONTAINER: KEY NOT FOUND";
            return rsc_e::BAD;
        }

        if (json["cnt"].is<JsonObject>() == false)
        {
            mResponseDescription = "INVALID CONTAINER: MUST BE A JSON OBJECT";
            return rsc_e::BAD;
        }
        
        JsonObject container = json["cnt"].as<JsonObject>();
        if (container.size() != mContainerLength)
        {
            if (mVersionState == rsc_e::UNCERTAIN_VERSION_CONFIG_INSTANCES)
            {
                mResponseDescription = 
                    "UNCERTAIN CONTAINER: VERSION DEFINES " + std::to_string(mContainerLength) + 
                    " KEYS WHILE CONTAINER HAS ONLY " + std::to_string(container.size());
                return mVersionState;
            }
            else
            {
                mResponseDescription = 
                    "INVALID CONTAINER: LENGTH MUST BE " + std::to_string(mContainerLength) + 
                    " FOR VERSION " + std::to_string(static_cast<uint8_t>(mVersion));
                return rsc_e::BAD_INVALID_VERSION_CONFIG_INSTANCES;
            }
        }

        for (auto config : container)
        {
            const auto retKey = Convert.ToJarvisKey(mVersion, config.key().c_str());
            if (retKey.first.ToCode() != Status::Code::GOOD)
            {
                mResponseDescription = "INVALID CONTAINER: \"" + std::string(config.key().c_str()) + "\" IS NOT DEFINED";
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            }/*컨테이너 안에서 사용할 수 있는 유효한 키(key) 값잆니다.*/

            if (config.value().is<JsonArray>() == false)
            {
                mResponseDescription = "INVALID CONTAINER: \"" + std::string(config.key().c_str()) + "\" IS NOT A JSON ARRAY";
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            }/*키(key)에 맵핑된 값이 유효한 JSON ARRAY 형식입니다.*/
            
            if ((config.value().isNull() == true) || (config.value().as<JsonArray>().size() == 0))
            {
                continue;
            }/*키(key)에 맵핑된 JSON ARRAY가 NULL이 아니며 길이가 0도 아닙니다.*/

            for (auto cin : config.value().as<JsonArray>())
            {
                if (cin.is<JsonObject>() == false)
                {
                    mResponseDescription = "INVALID CONTAINER: CONFIG INSTANCE MUST BE JSON OBJECT: \"" + std::string(config.key().c_str()) + "\"";
                    return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
                }
            }/*JSON ARRAY 내부의 Config Instance 형식이 유효한 JSON OBJECT 형식입니다.*/
            
            const auto retEmplace = mContainerKeySet.emplace(retKey.second);
            if (retEmplace.second == false)
            {
                mResponseDescription = "INVALID CONTAINER: \"" + std::string(config.key().c_str()) + "\" IS DUPLICATED";
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCES;
            }/*컨테이너에서 사용할 수 있는 유효한 키(key) 값이며, 설정 정보 내에서 유효합니다.*/
        }
        
        mResponseDescription = "GOOD";
        return rsc_e::GOOD;
    }

    std::pair<Status, prtcl_ver_e> MetaDataValidator::RetrieveProtocolVersion() const
    {
        if (mHasNoError == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mVersion);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE: JARVIS METADATA INVALID");
            return std::make_pair(Status(Status::Code::BAD), prtcl_ver_e::VERSEOIN_1);
        }
    }

    std::pair<Status, std::string> MetaDataValidator::RetrieveRequestID() const
    {
        if (mHasNoError == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mRequestID);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE: JARVIS METADATA INVALID");
            return std::make_pair(Status(Status::Code::BAD), std::string());
        }
    }

    std::pair<Status, std::set<cfg_key_e>> MetaDataValidator::RetrieveContainerKeys() const
    {
        if (mHasNoError == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mContainerKeySet);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE: JARVIS METADATA INVALID");
            return std::make_pair(Status(Status::Code::BAD), std::set<cfg_key_e>());
        }
    }


    prtcl_ver_e MetaDataValidator::SUPPORTED_VERSION[SUPPORTED_VERSION_LENGTH] =
    {
        prtcl_ver_e::VERSEOIN_1,
        prtcl_ver_e::VERSEOIN_2,
        prtcl_ver_e::VERSEOIN_3,
        prtcl_ver_e::VERSEOIN_4,
        prtcl_ver_e::VERSEOIN_5
        
    };
}}