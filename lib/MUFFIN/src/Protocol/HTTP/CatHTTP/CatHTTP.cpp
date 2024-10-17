/**
 * @file CatHTTP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 HTTP 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-09-23
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "CatHTTP.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Protocol/HTTP/Include/Helper.h"



namespace muffin { namespace http {


    CatHTTP* CatHTTP::GetInstanceOrNULL(CatM1& catM1)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) CatHTTP(catM1);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR CatHTTP");
                return nullptr;
            }
        }
        
        return mInstance;
    }

    CatHTTP* CatHTTP::GetInstanceOrNULL()
    {
        if (mInstance == nullptr)
        {
            ASSERT(false, "DEPENDANCY FOR CatM1 INSTANCE MUST BE INJECTED: CALL FUNCTION WITH CatM1 REFERENCE INSTEAD");
            return nullptr;
        }
        
        return mInstance;
    }

    
    /**
     * @todo Information Model에 MODLINK 모델 및 펌웨어 버전과 같은 정보를 Node
     *       형태로 표현한 다음 이를 토대로 user agengt 정보를 생성해야 합니다.
     */
    CatHTTP::CatHTTP(CatM1& catM1)
        : mCatM1(catM1)
        , mEnableCustomRequestHeader(true)
        , mEnableResponseHeaderOutput(false)
        , mSetSinkToCatFS(false)
        , mState(state_e::CONSTRUCTED)
    {
        mInitFlags.reset();
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }

    CatHTTP::~CatHTTP()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status CatHTTP::Init(const network::lte::pdp_ctx_e pdp, const network::lte::ssl_ctx_e ssl, const bool customRequestHeader, const bool outputResponse)
    {
        ASSERT((mState != state_e::INITIALIZED), "REINITIALIZATION IS FORBIDDEN");

        Status ret = Status(Status::Code::UNCERTAIN);

        if (mInitFlags.test(init_flag_e::INITIALIZED_PDP) == false)
        {
            ret = setPdpContext(pdp);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET PDP CONTEXT: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_PDP);
        }
        
        if (mInitFlags.test(init_flag_e::INITIALIZED_SSL) == false)
        {
            ret = setSslContext(ssl);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET SSL CONTEXT: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_SSL);
        }

        if (mInitFlags.test(init_flag_e::INITIALIZED_REQ) == false)
        {
            ret = setCustomRequestHeader(customRequestHeader);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET REQUEST HEADER: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_REQ);
        }

        if (mInitFlags.test(init_flag_e::INITIALIZED_RES) == false)
        {
            ret = setResponseHeaderOutput(outputResponse);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET RESPONSE OUTPUT: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_RES);
        }

        LOG_INFO(logger, "Initialized successfully");
        mInitFlags.set(init_flag_e::INITIALIZED_ALL);
        mState = state_e::INITIALIZED;
        return Status(Status::Code::GOOD);

    INIT_FAILED:
        mState = state_e::INIT_FAILED;
        return ret;
    }

    Status CatHTTP::GET(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        ASSERT((0 < strlen(header.c_str()) && strlen(header.c_str()) < 2049), "INVALID HEADER LENGTH");
        ASSERT((0 < timeout), "INVALID TIMEOUT VALUE");

        header.UpdateParamter(parameter.c_str());
        Status ret = setRequestURL(header.GetURL(), timeout);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET REQUEST URL:%s, %s",
                header.GetURL().c_str(), ret.c_str());
            return ret;
        }

        constexpr uint8_t BUFFER_SIZE = 32;
        
        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPGET=%u,%u", timeout, strlen(header.c_str()));
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");

        const uint32_t timeoutMillis = timeout * 1000;
        std::string rxd;

        ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REQUEST GET: %s", ret.c_str());
            return ret;
        }

        ret = readUntilCONNECT(timeoutMillis, &rxd);
        LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            Status cmeErrorCode = processCmeErrorCode(rxd);
            LOG_ERROR(logger, "FAILED TO REQUEST GET: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return cmeErrorCode;
        }

        ret = mCatM1.Execute(header.c_str());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REQUEST GET: %s", ret.c_str());
            return ret;
        }
        rxd.clear();

        ret = readUntilRSC(timeoutMillis, &rxd);
        LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            Status cmeErrorCode = processCmeErrorCode(rxd);
            LOG_ERROR(logger, "FAILED TO REQUEST GET: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return cmeErrorCode;
        }

        std::vector<size_t> vecDelimiter;
        vecDelimiter.emplace_back(rxd.find(' '));
        if (vecDelimiter.front() == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        size_t position = std::string::npos;
        do
        {
            position = rxd.find(',', vecDelimiter.back() + 1);
            if (position != std::string::npos && position != rxd.length())
            {
                vecDelimiter.emplace_back(position);
            }
        } while (position != std::string::npos);
        vecDelimiter.emplace_back(rxd.find('\r', vecDelimiter.back() + 1));

        constexpr uint8_t MINIMUM_DELIMITER_COUNT = 3;
        constexpr uint8_t MAXIMUM_DELIMITER_COUNT = 4;
        if (vecDelimiter.size() != MINIMUM_DELIMITER_COUNT &&
            MAXIMUM_DELIMITER_COUNT != vecDelimiter.size())
        {
            LOG_ERROR(logger, "INVALID NUMBER OF DELIMITER: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        std::vector<size_t>::iterator it = vecDelimiter.begin();
        const std::string strCME = rxd.substr(*it + 1, *(it + 1) - *it - 1);
        const std::string strRSC = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strLen = (it + 1) == vecDelimiter.end() ?
            "" : 
            rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);

        const int32_t rxdCME = ConvertStringToInt32(strCME.c_str());
        const http_rsc_e rxdRSC = ConvertInt32ToRSC(ConvertStringToInt32(strRSC.c_str()));
        const int32_t rxdLen = strLen == "" ? 
            -1 : 
            ConvertStringToInt32(strLen.c_str());

        if (rxdCME == INT32_MAX || rxdRSC == http_rsc_e::UNDEFINED_RSC || rxdLen == INT32_MAX)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        switch (rxdRSC)
        {
        case http_rsc_e::OK:
            LOG_INFO(logger, "RSC: %s, ContentLength: %d", ConvertRscToString(rxdRSC), rxdLen);
            if (mSetSinkToCatFS == true)
            {
                goto SINK_TO_CatFS;
            }
            return Status(Status::Code::GOOD);
        case http_rsc_e::FORBIDDEN:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_NOT_ALLOWED);
        case http_rsc_e::NOT_FOUND:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_NOT_FOUND);
        case http_rsc_e::CONFLICT:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_INTERRUPTED);
        case http_rsc_e::LENGTH_REQUIRED:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_HEADER_INVALID);
        case http_rsc_e::INTERNAL_SERVER_ERRROR:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_SERVER_HALTED);
        default:
            LOG_ERROR(logger, "UNDEFINED RSC");
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

    SINK_TO_CatFS:
        return sendResponse2CatFS();
    }

    Status CatHTTP::POST(RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        ASSERT((0 < strlen(header.c_str()) && strlen(header.c_str()) < 2049), "INVALID HEADER LENGTH");
        ASSERT((0 < strlen(body.c_str()) && strlen(body.c_str()) < 1021953), "INVALID BODY LENGTH");
        ASSERT((0 < timeout), "INVALID TIMEOUT VALUE");

        Status ret = setRequestURL(header.GetURL(), timeout);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET REQUEST URL:%s, %s",
                header.GetURL().c_str(), ret.c_str());
            return ret;
        }

        constexpr uint8_t BUFFER_SIZE = 64;
        header.SetContentLength(strlen(body.c_str()));
        header.SetContentType(body.GetContentType());

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPPOST=%u,%u,%u",
            (strlen(header.c_str()) + strlen(body.c_str())),
            timeout, timeout);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");

        const uint32_t timeoutMillis = timeout * 1000;
        std::string rxd;

        ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REQUEST POST: %s", ret.c_str());
            return ret;
        }

        ret = readUntilCONNECT(timeoutMillis, &rxd);
        LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            Status cmeErrorCode = processCmeErrorCode(rxd);
            LOG_ERROR(logger, "FAILED TO REQUEST POST: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return cmeErrorCode;
        }

        LOG_DEBUG(logger, "Content: %s", (header.ToString() + body.ToString()).c_str());
        LOG_DEBUG(logger, "Content: %u", (header.ToString() + body.ToString()).length());
        ret = mCatM1.Execute(header.ToString() + body.ToString());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REQUEST POST: %s", ret.c_str());
            return ret;
        }
        rxd.clear();

        ret = readUntilRSC(timeoutMillis, &rxd);
        LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            Status cmeErrorCode = processCmeErrorCode(rxd);
            LOG_ERROR(logger, "FAILED TO REQUEST POST: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return cmeErrorCode;
        }

        std::vector<size_t> vecDelimiter;
        vecDelimiter.emplace_back(rxd.find(' '));
        if (vecDelimiter.front() == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        size_t position = std::string::npos;
        do
        {
            position = rxd.find(',', vecDelimiter.back() + 1);
            if (position != std::string::npos && position != rxd.length())
            {
                vecDelimiter.emplace_back(position);
            }
        } while (position != std::string::npos);
        vecDelimiter.emplace_back(rxd.find('\r', vecDelimiter.back() + 1));

        constexpr uint8_t MINIMUM_DELIMITER_COUNT = 3;
        constexpr uint8_t MAXIMUM_DELIMITER_COUNT = 4;
        if (vecDelimiter.size() != MINIMUM_DELIMITER_COUNT &&
            MAXIMUM_DELIMITER_COUNT != vecDelimiter.size())
        {
            LOG_ERROR(logger, "INVALID NUMBER OF DELIMITER: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        std::vector<size_t>::iterator it = vecDelimiter.begin();
        const std::string strCME = rxd.substr(*it + 1, *(it + 1) - *it - 1);
        const std::string strRSC = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strLen = (it + 1) == vecDelimiter.end() ?
            "" : 
            rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);

        const int32_t rxdCME = ConvertStringToInt32(strCME.c_str());
        const http_rsc_e rxdRSC = ConvertInt32ToRSC(ConvertStringToInt32(strRSC.c_str()));
        const int32_t rxdLen = strLen == "" ? 
            -1 : 
            ConvertStringToInt32(strLen.c_str());

        if (rxdCME == INT32_MAX || rxdRSC == http_rsc_e::UNDEFINED_RSC || rxdLen == INT32_MAX)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        switch (rxdRSC)
        {
        case http_rsc_e::OK:
            LOG_INFO(logger, "RSC: %s, ContentLength: %d", ConvertRscToString(rxdRSC), rxdLen);
            if (mSetSinkToCatFS == true)
            {
                goto SINK_TO_CatFS;
            }
            return Status(Status::Code::GOOD);
        case http_rsc_e::FORBIDDEN:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_NOT_ALLOWED);
        case http_rsc_e::NOT_FOUND:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_NOT_FOUND);
        case http_rsc_e::CONFLICT:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_INTERRUPTED);
        case http_rsc_e::LENGTH_REQUIRED:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_REQUEST_HEADER_INVALID);
        case http_rsc_e::INTERNAL_SERVER_ERRROR:
            LOG_INFO(logger, "RSC: %s", ConvertRscToString(rxdRSC));
            return Status(Status::Code::BAD_SERVER_HALTED);
        default:
            LOG_ERROR(logger, "UNDEFINED RSC");
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

    SINK_TO_CatFS:
        return sendResponse2CatFS();
    }

    Status CatHTTP::Retrieve(std::string* response)
    {
        ASSERT((response != nullptr), "RESPONSE VECTOR CANNOT BE NULL POINTER");
        ASSERT((response->size() == 0), "RESPONSE VECTOR MUST BE EMPTY");
        ASSERT((mSetSinkToCatFS == false), "RESPONSE IS SET TO BE SAVED IN THE CatFS");

        constexpr uint8_t BUFFER_SIZE = 32;
        constexpr uint8_t TIMEOUT_IN_SECOND = 60;

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPREAD=%u", TIMEOUT_IN_SECOND);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = TIMEOUT_IN_SECOND * 1000;
        uint32_t errorCode;
        std::string rxd;
        size_t pos;
        size_t length;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }

        ret = readUntilCONNECT(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }
        rxd.clear();

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }

        *response = rxd;
        rxd.clear();

        ret = readUntilRSC(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }

        pos = rxd.find(" ");
        if (pos == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        
        length = rxd.find("\r", pos) - pos - 1;
        errorCode = ConvertStringToUInt32(rxd.substr(pos + 1, length).c_str());
        if (errorCode == UINT32_MAX)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        response->erase(response->find("OK"));
        while (*--response->end() == '\r' || *--response->end() == '\n')
        {
            response->erase(--response->end());
        }
        return convertErrorCode(errorCode);

    CME_ERROR:
        response->clear();
        Status cmeErrorCode = processCmeErrorCode(rxd);
        LOG_ERROR(logger, "FAILED TO SEND RESPONSE TO ESP32: %s: %s", 
            ret.c_str(), processCmeErrorCode(rxd).c_str());
        return cmeErrorCode;
    }

    void CatHTTP::SetSinkToCatFS(const bool save2CatFS)
    {
        mSetSinkToCatFS = save2CatFS;
    }

    Status CatHTTP::setPdpContext(const network::lte::pdp_ctx_e pdp)
    {
        constexpr uint8_t BUFFER_SIZE = 32;

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPCFG=\"contextid\",%u", static_cast<uint8_t>(pdp));
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET PDP CONTEXT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Set PDP context: #%u", static_cast<uint8_t>(pdp));
            mContextPDP = pdp;
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET PDP CONTEXT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatHTTP::setSslContext(const network::lte::ssl_ctx_e ssl)
    {
        constexpr uint8_t BUFFER_SIZE = 32;

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPCFG=\"sslctxid\",%u", static_cast<uint8_t>(ssl));
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET SSL CONTEXT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Set SSL context: #%u", static_cast<uint8_t>(ssl));
            mContextSSL = ssl;
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET SSL CONTEXT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatHTTP::setCustomRequestHeader(const bool enable)
    {
        constexpr uint8_t BUFFER_SIZE = 32;

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPCFG=\"requestheader\",%u", static_cast<uint8_t>(enable));
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET CUSTOM REQUEST HEADER: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Set custom request header: %s", enable ? "ENABLED" : "DISABLED");
            mEnableCustomRequestHeader = enable;
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET CUSTOM REQUEST HEADER: %s", ret.c_str());
            return ret;
        }
    }

    Status CatHTTP::setResponseHeaderOutput(const bool turnOff)
    {
        constexpr uint8_t BUFFER_SIZE = 32;
        
        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPCFG=\"responseheader\",%u", static_cast<uint8_t>(turnOff));
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET RESPONSE HEADER OUTPUT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Set response header output: %s", turnOff ? "TURNED OFF" : "TURNED ON");
            mEnableResponseHeaderOutput = turnOff;
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET RESPONSE HEADER OUTPUT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatHTTP::sendResponse2CatFS()
    {
        ASSERT((mSetSinkToCatFS == true), "CatFS STORAGE OPTION MUST BE TURNED ON");

        constexpr uint8_t BUFFER_SIZE = 64;
        constexpr uint8_t TIMEOUT_IN_SECOND = 60;

        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPREADFILE=\"http_response_file\",%u", TIMEOUT_IN_SECOND);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = TIMEOUT_IN_SECOND * 1000;
        uint32_t errorCode;
        std::string rxd;
        size_t pos;
        size_t length;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }
        rxd.clear();

        ret = readUntilRSC(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            goto CME_ERROR;
        }

        pos = rxd.find(" ");
        if (pos == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        length = rxd.find("\r", pos) - pos - 1;
        errorCode = ConvertStringToUInt32(rxd.substr(pos + 1, length).c_str());
        if (errorCode == UINT32_MAX)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        LOG_INFO(logger, "Saved into CatFS with file name: \"http_response_file\"");
        return convertErrorCode(errorCode);

    CME_ERROR:
        Status cmeErrorCode = processCmeErrorCode(rxd);
        LOG_ERROR(logger, "FAILED TO SEND RESPONSE TO ESP32: %s: %s", 
            ret.c_str(), processCmeErrorCode(rxd).c_str());
        return cmeErrorCode;
    }

    Status CatHTTP::setRequestURL(const std::string& url, const uint16_t timeout)
    {
        ASSERT(
            (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://"),
            "SCHEME MUST BE PROVIDED"
        );
        ASSERT((1 < url.length() && url.length() < 700), "INVALID URL LENGTH");
        ASSERT((1 < timeout && timeout < 65535), "INVALID TIMEOUT");

        constexpr uint8_t BUFFER_SIZE = 32;
        
        char command[BUFFER_SIZE];
        memset(command, '\0', sizeof(command));
        sprintf(command, "AT+QHTTPURL=%u,%u", url.length(), timeout);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET REQUEST URL: %s", ret.c_str());
            return ret;
        }

        ret = readUntilCONNECT(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET REQUEST URL: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }

        ret = mCatM1.Execute(url);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET REQUEST URL: %s", ret.c_str());
            return ret;
        }
        rxd.clear();

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Set request url: %s", url.c_str());
            return ret;
        }
        else
        {
            Status cmeErrorCode = processCmeErrorCode(rxd);
            LOG_ERROR(logger, "FAILED TO SET REQUEST HEADER: %s: %s", 
                ret.c_str(), processCmeErrorCode(rxd).c_str());
            return cmeErrorCode;
        }
    }

    Status CatHTTP::readUntilCONNECT(const uint32_t timeoutMillis, std::string* rxd)
    {
        ASSERT((rxd != nullptr), "OUTPUT PARAMETER MUST NOT BE A NULLPTR");
        
        const uint32_t startMillis = millis();

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mCatM1.GetAvailableBytes() > 0)
            {
                int16_t value = mCatM1.Read();
                if (value == -1)
                {
                    LOG_WARNING(logger, "FAILED TO TAKE MUTEX OR NO DATA AVAILABLE");
                    continue;
                }
                *rxd += value;

                if (rxd->find("CONNECT\r\n") != std::string::npos)
                {
                    return Status(Status::Code::GOOD);
                }
                else if (rxd->find("ERROR\r\n") != std::string::npos)
                {
                    return Status(Status::Code::BAD_DEVICE_FAILURE);
                }
                else
                {
                    continue;
                }
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    Status CatHTTP::readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd)
    {
        ASSERT((rxd != nullptr), "OUTPUT PARAMETER MUST NOT BE A NULLPTR");
        
        const uint32_t startMillis = millis();

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mCatM1.GetAvailableBytes() > 0)
            {
                int16_t value = mCatM1.Read();
                if (value == -1)
                {
                    LOG_WARNING(logger, "FAILED TO TAKE MUTEX OR NO DATA AVAILABLE");
                    continue;
                }
                *rxd += value;

                if (rxd->find("OK\r\n") != std::string::npos)
                {
                    return Status(Status::Code::GOOD);
                }
                else if (rxd->find("ERROR\r\n") != std::string::npos)
                {
                    return Status(Status::Code::BAD_DEVICE_FAILURE);
                }
                else
                {
                    continue;
                }
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    Status CatHTTP::readUntilRSC(const uint32_t timeoutMillis, std::string* rxd) 
    {
        ASSERT((rxd != nullptr), "OUTPUT PARAMETER MUST NOT BE A NULLPTR");

        const uint32_t startMillis = millis();

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mCatM1.GetAvailableBytes() > 0)
            {
                int16_t value = mCatM1.Read();
                if (value == -1)
                {
                    LOG_WARNING(logger, "FAILED TO TAKE MUTEX OR NO DATA AVAILABLE");
                    continue;
                }
                *rxd += value;
            }

            if (rxd->find("+QHTTPGET: ")  != std::string::npos ||
                rxd->find("+QHTTPPOST: ") != std::string::npos ||
                rxd->find("+QHTTPREAD: ") != std::string::npos ||
                rxd->find("+QHTTPREADFILE: ") != std::string::npos)
            {
                if (rxd->find_last_of("\r\n") != std::string::npos)
                {
                    rxd->erase(rxd->find_last_of("\r\n"));
                }
                return Status(Status::Code::GOOD);
            }
            else if (rxd->find("+CME ERROR: ") != std::string::npos)
            {
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    Status CatHTTP::processCmeErrorCode(const std::string& rxd)
    {
        const std::string cmeErrorIndicator = "+CME ERROR: ";
        const size_t cmeStartPosition  = rxd.find(cmeErrorIndicator) + cmeErrorIndicator.length();
        const size_t cmeFinishPosition = rxd.find("\r", cmeStartPosition);
        if (cmeStartPosition == std::string::npos || cmeFinishPosition == std::string::npos)
        {
            LOG_DEBUG(logger, "INVALID CME ERROR CODE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const size_t cmeErrorCodeLength = cmeFinishPosition - cmeStartPosition;
        const std::string cmeErrorCodeString = rxd.substr(cmeStartPosition, cmeErrorCodeLength);
        const uint32_t cmeErrorCode = ConvertStringToUInt32(cmeErrorCodeString.c_str());
        return convertErrorCode(cmeErrorCode);
    }

    Status CatHTTP::convertErrorCode(const uint16_t errorCode)
    {
        switch (errorCode)
        {
        case 0:
            LOG_INFO(logger, "NO ERROR");
            return Status(Status::Code::GOOD);
        case 701:
            LOG_ERROR(logger, "UNKNOWN ERROR");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        case 702:
            LOG_ERROR(logger, "TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 703:
            LOG_ERROR(logger, "BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 704:
            LOG_ERROR(logger, "UART BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 705:
            LOG_ERROR(logger, "NO GET/POST REQUESTS");
            return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
        case 706:
            LOG_ERROR(logger, "NETWORK BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 707:
            LOG_ERROR(logger, "NETWORK OPEN FAILED");
            return Status(Status::Code::BAD_NO_COMMUNICATION);
        case 708:
            LOG_ERROR(logger, "NO NETWORK CONFIGURATION");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        case 709:
            LOG_ERROR(logger, "NETWORK DEACTIVATED");
            return Status(Status::Code::BAD_NO_COMMUNICATION);
        case 710:
            LOG_ERROR(logger, "NETWORK ERROR");
            return Status(Status::Code::BAD_NO_COMMUNICATION);
        case 711:
            LOG_ERROR(logger, "URL ERROR");
            return Status(Status::Code::BAD_TCP_ENDPOINT_URL_INVALID);
        case 712:
            LOG_ERROR(logger, "EMPTY URL");
            return Status(Status::Code::BAD_TCP_ENDPOINT_URL_INVALID);
        case 713:
            LOG_ERROR(logger, "IP ADDRESS ERROR");
            return Status(Status::Code::BAD_TCP_ENDPOINT_URL_INVALID);
        case 714:
            LOG_ERROR(logger, "DNS ERROR");
            return Status(Status::Code::BAD_TCP_ENDPOINT_URL_INVALID);
        case 715:
            LOG_ERROR(logger, "SOCKET CREATE ERROR");
            return Status(Status::Code::BAD_TCP_INTERNAL_ERROR);
        case 716:
            LOG_ERROR(logger, "SOCKET CONNECT ERROR");
            return Status(Status::Code::BAD_NOT_CONNECTED);
        case 717:
            LOG_ERROR(logger, "SOCKET READ ERROR");
            return Status(Status::Code::BAD_TCP_INTERNAL_ERROR);
        case 718:
            LOG_ERROR(logger, "SOCKET WRITE ERROR");
            return Status(Status::Code::BAD_TCP_INTERNAL_ERROR);
        case 719:
            LOG_ERROR(logger, "SOCKET CLOSED");
            return Status(Status::Code::BAD_CONNECTION_CLOSED);
        case 720:
            LOG_ERROR(logger, "DATA ENCODE ERROR");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        case 721:
            LOG_ERROR(logger, "DATA DECODE ERROR");
            return Status(Status::Code::BAD_DECODING_ERROR);
        case 722:
            LOG_ERROR(logger, "READ TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 723:
            LOG_ERROR(logger, "RESPONSE FAILED");
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        case 724:
            LOG_ERROR(logger, "INCOMING CALL BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 725:
            LOG_ERROR(logger, "VOID CALL BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 726:
            LOG_ERROR(logger, "INPUT TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 727:
            LOG_ERROR(logger, "WAIT DATA TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 728:
            LOG_ERROR(logger, "WAIT RESPONSE TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 729:
            LOG_ERROR(logger, "MEMORY ALLOCATION FAILED");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 730:
            LOG_ERROR(logger, "INVALID PARAMETER");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        default:
            // LOG_DEBUG(logger, "INVALID CME ERROR CODE: %s", rxd.c_str());
            // return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            assert(false);
        }
    }


    CatHTTP* CatHTTP::mInstance = nullptr;
}}