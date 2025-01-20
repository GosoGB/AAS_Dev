/**
 * @file LwipHTTP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LWIP TCP/IP stack 기반의 HTTP 클라이언트 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo Information Model에 MODLINK 모델 및 펌웨어 버전과 같은 정보를 Node 
 *       형태로 표현한 다음 이를 토대로 user agengt 정보를 생성해야 합니다.
 */



#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "LwipHTTP.h"
#include "IM/Custom/Constants.h"
#include "Network/Ethernet/Ethernet.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "Protocol/HTTP/Include/Helper.h"
#include "Protocol/Certs.h"



namespace muffin { namespace http {
    
    Status LwipHTTP::Init()
    {
        mClientSecure.setCACert(ROOT_CA_CRT);
        return Status(Status::Code::GOOD);
    }
    
    Status LwipHTTP::GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            return getHTTP(header, parameter, timeout);

        case http_scheme_e::HTTPS:
            return getHTTPS(header, parameter, timeout);

        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            return postHTTP(header, body, timeout);

        case http_scheme_e::HTTPS:
            return postHTTPS(header, body, timeout);

        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            return postHTTP(header, parameter, timeout);
            
        case http_scheme_e::HTTPS:
            return postHTTPS(header, parameter, timeout);
            
        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
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
        if (mClient.connect(header.GetHost().c_str(), header.GetPort()) == false)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT: %s:%u", header.GetHost().c_str(), header.GetPort());
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }

        Status ret = header.UpdateParamter(parameter.ToString().c_str());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UPDATE PARAMETER: %s", parameter.ToString().c_str());
            return ret;
        }
        
        mClient.print(header.ToString().c_str());
        ret = processResponseHeader(timeout);
        

        if (millis() - startedMillis > timeout) 
        {
            LOG_ERROR(logger,"Client Timeout !");
            return Status(Status::Code::BAD_TIMEOUT);
        }

        std::string body = getHttpBody((result.c_str()));
        LOG_WARNING(logger,"[body] : %s",body.c_str());

        mClient.stop();
        return ret; 
    }

    Status LwipHTTP::getHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
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
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }

        return ret; 
    }

    Status LwipHTTP::postHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClient.connect(header.GetHost().c_str(),header.GetPort()))
        {
            header.UpdateParamter(parameter.ToString().c_str());
            mClient.print(header.ToString().c_str());

            unsigned long timeout = millis();
        
            while (mClient.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClient.available())
            {
                result += mClient.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
            mClient.stop();
        }
        else
        {
            LOG_ERROR(logger,"FAIL TO CONNECT SERVER");
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
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
            mClient.print((headerStr + bodyStr).c_str());
            unsigned long timeout = millis();
        
            while (mClient.available() == 0) 
            {
                if (millis() - timeout > 5000) 
                {
                    LOG_ERROR(logger,"Client Timeout !");
                    return Status(Status::Code::BAD_TIMEOUT);
                }
            }

            std::string result;
            while (mClient.available())
            {
                result += mClient.read();
            }

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
            mClient.stop();
        } 
        else
        {
            LOG_ERROR(logger,"FAIL TO CONNECT SERVER");
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }

        return ret;
    }

    Status LwipHTTP::postHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::GOOD);
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
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

            mResponseData.clear();
            mResponseData.shrink_to_fit();

            mResponseData = getHttpBody((result.c_str()));
            LOG_WARNING(logger,"[body] : %s",mResponseData.c_str());
            mClientSecure.stop();
        }
        else
        {
            LOG_ERROR(logger,"FAIL TO CONNECT SERVER");
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
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
            mClientSecure.stop();
        }
        else
        {
            LOG_ERROR(logger,"FAIL TO CONNECT SERVER");
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }
        
        return ret;
    }

    Status LwipHTTP::processResponseHeader(const uint16_t timeout)
    {
        const uint32_t startedMillis = millis();
        const uint8_t size = 128;
        uint8_t buffer[size] = {0};
        uint8_t idx = 0;
        bool isEnded = false;

        while ((millis() - startedMillis) < (timeout * SECOND_IN_MILLIS))
        {
            while (mClient.available() > 0)
            {
                const int value = mClient.read();
                if (value < 0)
                {
                    continue;
                }
                
                if (value == '\n')
                {
                    buffer[idx] = '\0';
                    break;
                }
                else if (value != '\r')
                {
                    buffer[idx++] = value;
                }
            }

            idx = 0;
        }
    

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
}}