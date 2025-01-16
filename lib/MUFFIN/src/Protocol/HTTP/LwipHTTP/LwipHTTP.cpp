/**
 * @file LwipHTTP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 HTTP 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include "LwipHTTP.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "LwipHTTP.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/HTTP/Include/Helper.h"
#include "Protocol/Certs.h"



namespace muffin { namespace http {


    LwipHTTP* LwipHTTP::CreateInstanceOrNULL()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) LwipHTTP();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR LwipHTTP");
                return nullptr;
            }
        }
        
        return mInstance;
    }

    LwipHTTP& LwipHTTP::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    
    /**
     * @todo Information Model에 MODLINK 모델 및 펌웨어 버전과 같은 정보를 Node
     *       형태로 표현한 다음 이를 토대로 user agengt 정보를 생성해야 합니다.
     */
    LwipHTTP::LwipHTTP()
    {
    }

    LwipHTTP::~LwipHTTP()
    {
    }

    Status LwipHTTP::Init()
    {
        xSemaphore = xSemaphoreCreateMutex();
        if (xSemaphore == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE SEMAPHORE");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        mClientSecure.setCACert(certificates);
        return Status(Status::Code::GOOD);
    }
    
    Status LwipHTTP::GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        switch (header.GetSchem())
        {
        case http_scheme_e::HTTP:
            ret = getHTTP(header, parameter, timeout);
            break;
        case http_scheme_e::HTTPS:
            ret = getHTTPS(header, parameter, timeout);
            break;
        default:
            break;
        }

        return ret;
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        switch (header.GetSchem())
        {
        case http_scheme_e::HTTP:
            ret = postHTTP(header, body, timeout);
            break;
        case http_scheme_e::HTTPS:
            ret = postHTTPS(header, body, timeout);
            break;
        default:
            break;
        }

        return ret;
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        switch (header.GetSchem())
        {
        case http_scheme_e::HTTP:
            ret = postHTTP(header, parameter, timeout);
            break;
        case http_scheme_e::HTTPS:
            ret = postHTTPS(header, parameter, timeout);
            break;
        default:
            break;
        }

        return ret;
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, std::string* response)
    {
        Status ret = Status(Status::Code::GOOD);
        *response = mResponseData;

        mResponseData.clear();
        mResponseData.shrink_to_fit();
        
        return ret;
    }

    INetwork* LwipHTTP::RetrieveNIC()
    {
        return static_cast<INetwork*>(ethernet);
    }

    Status LwipHTTP::getHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        
        if (mClient.connect(header.GetHost().c_str(),header.GetPort()))
        {
            LOG_INFO(logger,"CONNECT!");
            header.UpdateParamter(parameter.ToString().c_str());
            mClientSecure.print(header.ToString().c_str());

            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClientSecure.available())
            {
                result += mClientSecure.read();
            }

            std::string body = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",body.c_str());
        }

        return ret; 
    }

    Status LwipHTTP::getHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        mClientSecure.setCACert(certificates);
        
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
            LOG_INFO(logger,"CONNECT!");
            header.UpdateParamter(parameter.ToString().c_str());

            LOG_INFO(logger,"header : %s",header.ToString().c_str());
            mClientSecure.print(header.ToString().c_str());

            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            LOG_WARNING(logger,"mClientSecure.available() : %u",mClientSecure.available());
            while (mClientSecure.available())
            {
                result += mClientSecure.read(); 
            }
            LOG_WARNING(logger,"[AFTER] mClientSecure.available() : %u",mClientSecure.available());
            LOG_WARNING(logger,"[body] : %s",result.c_str());
            // Status::Code::GOOD_MORE_DATA; -> 반환시 나머지 호출해서 합치는

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            mClientSecure.stop();
        }
        else
        {
            LOG_ERROR(logger,"FAIL TO CONNECT SERVER");
            return Status(Status::Code::BAD);
        }

        return ret; 
    }

    Status LwipHTTP::postHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClient.connect(header.GetHost().c_str(),header.GetPort()))
        {
            header.UpdateParamter(parameter.ToString().c_str());
            LOG_INFO(logger, "Request msg : %s", header.ToString().c_str());
            mClient.print(header.ToString().c_str());

            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClientSecure.available())
            {
                result += mClientSecure.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
        }

        return ret;
    }

    Status LwipHTTP::postHTTP(RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClient.connect(header.GetHost().c_str(),header.GetPort()))
        {
            header.SetContentLength(strlen(body.ToString().c_str()));
            header.SetContentType(body.GetContentType());
            std::string headerStr = header.ToString();
            std::string bodyStr = body.ToString();
            LOG_INFO(logger, "Request msg : %s", (headerStr + bodyStr).c_str());
            mClient.print((headerStr + bodyStr).c_str());
            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClientSecure.available())
            {
                result += mClientSecure.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
        }   

        return ret;
    }

    Status LwipHTTP::postHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
            header.UpdateParamter(parameter.ToString().c_str());
            LOG_INFO(logger, "Request msg : %s", header.ToString().c_str());
            mClientSecure.print(header.ToString().c_str());

            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClientSecure.available())
            {
                result += mClientSecure.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
        }

        return ret;
    }

    Status LwipHTTP::postHTTPS(RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
            header.SetContentLength(strlen(body.ToString().c_str()));
            header.SetContentType(body.GetContentType());
            std::string headerStr = header.ToString();
            std::string bodyStr = body.ToString();
            LOG_INFO(logger, "Request msg : %s", (headerStr + bodyStr).c_str());
            mClientSecure.print((headerStr + bodyStr).c_str());
            unsigned long timeout = millis();
        
            while (mClientSecure.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClientSecure.available())
            {
                result += mClientSecure.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
        }
        
        return ret;
    }

    std::string LwipHTTP::getHttpBody(const std::string& payload) 
    {
        bool HeaderExist = false;
        bool Chunked = false;
        std::string header = "";
        std::string body = "";

        if (payload.find("HTTP") == 0) 
        {
            HeaderExist = true;
        }

        if (HeaderExist) 
        {
            size_t idx = payload.find("\r\n\r\n"); // header와 body를 나누는 부분
            if (idx != std::string::npos) 
            {
                header = payload.substr(0, idx + 2);
                body = payload.substr(idx + 4);

                std::string key = "Transfer-Encoding";
                size_t keyIndex = header.find(key);
                if (keyIndex != std::string::npos) 
                {
                    size_t startIndex = header.find(": ", keyIndex);
                    size_t stopIndex = header.find("\r\n", keyIndex);
                    if (startIndex != std::string::npos && stopIndex != std::string::npos) 
                    {
                        if (header.substr(startIndex + 2, stopIndex - startIndex - 2).find("chunked") != std::string::npos) 
                        {
                            Chunked = true;
                        }
                    }
                }
            }
        } 
        else
        {
            body = payload;
        }

        if (Chunked || !HeaderExist) 
        {
            std::string decodedBody = "";
            while (!body.empty()) 
            {
                // 청크 크기 추출
                size_t chunkSizeEnd = body.find("\r\n");
                if (chunkSizeEnd == std::string::npos) 
                {
                    break; // 잘못된 형식
                }
                std::string chunkSizeStr = body.substr(0, chunkSizeEnd);
                int chunkSize = static_cast<int>(strtol(chunkSizeStr.c_str(), nullptr, 16));

                if (chunkSize == 0) 
                {
                    break; // 마지막 청크
                }

                // 청크 데이터 추출
                size_t chunkDataStart = chunkSizeEnd + 2; // "\r\n" 이후
                size_t chunkDataEnd = chunkDataStart + chunkSize;
                if (chunkDataEnd > body.size()) 
                {
                    break; // 잘못된 형식
                }
                decodedBody += body.substr(chunkDataStart, chunkSize);

                // 다음 청크로 이동
                body = body.substr(chunkDataEnd + 2); // "\r\n" 이후
            }

            body = decodedBody; // 디코딩된 바디로 대체
        }
        return body;
    }






    std::pair<Status, size_t> LwipHTTP::TakeMutex()
    {
        if (xSemaphoreTake(xSemaphore, 5000)  != pdTRUE)
        {
            LOG_WARNING(logger, "FAILED TO TAKE MUTEX FOR LWIP TRY LATER.");
            return std::make_pair(Status(Status::Code::BAD_TOO_MANY_OPERATIONS), mMutexHandle);
        }

        ++mMutexHandle;
        return std::make_pair(Status(Status::Code::GOOD), mMutexHandle);
    }

    Status LwipHTTP::ReleaseMutex()
    {
        xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }


    LwipHTTP* LwipHTTP::mInstance = nullptr;
}}