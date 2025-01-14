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
        const http_scheme_e schme = header.GetSchem();
        Status ret = Status(Status::Code::UNCERTAIN);
        switch (schme)
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
        const http_scheme_e schme = header.GetSchem();
        // switch (schme)
        // {
        // case http_scheme_e::HTTP:
        //     ret = postHTTP(header, body, timeout);
        //     break;
        // case http_scheme_e::HTTPS:
        //     ret = postHTTPS(header, body, timeout);
        //     break;
        // default:
        //     break;
        // }

        return ret;
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        // const http_scheme_e schme = header.GetSchem();
        // switch (schme)
        // {
        // case http_scheme_e::HTTP:
        //     ret = postHTTP(header, parameter, timeout);
        //     break;
        // case http_scheme_e::HTTPS:
        //     ret = postHTTPS(header, parameter, timeout);
        //     break;
        // default:
        //     break;
        // }

        return ret;
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, std::string* response)
    {
        Status ret = Status(Status::Code::GOOD);
        *response = mResponseData;
        return ret;
    }

    Status LwipHTTP::getHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        Status ret = Status(Status::Code::UNCERTAIN);
        header.UpdateParamter(parameter.ToString().c_str());
        
        if (mClient.connect(header.GetHost().c_str(),header.GetPort()))
        {
            LOG_INFO(logger,"CONNECT!");
            
            std::string str = header.ToString();
            mClientSecure.print(str.c_str());

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
        header.UpdateParamter(parameter.ToString().c_str());
        
        if (mClientSecure.connect(header.GetHost().c_str(),header.GetPort()))
        {
            LOG_INFO(logger,"CONNECT!");
            
            std::string str = header.ToString();
            mClientSecure.print(str.c_str());

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

    Status LwipHTTP::convertErrorCode(const uint16_t errorCode)
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


    LwipHTTP* LwipHTTP::mInstance = nullptr;
}}