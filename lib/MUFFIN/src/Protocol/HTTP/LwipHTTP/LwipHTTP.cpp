/**
 * @file LwipHTTP.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LWIP TCP/IP stack 기반의 HTTP 클라이언트 클래스를 정의합니다.
 * 
 * @date 2025-01-21
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo Information Model에 MODLINK 모델 및 펌웨어 버전과 같은 정보를 Node 
 *       형태로 표현한 다음 이를 토대로 user agengt 정보를 생성해야 합니다.
 * 
 * @todo ESP32FS 초기화 부분에서 mResponsePath에 있는 파일을 모두 삭제하게 만들어야 합니다.
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

        // case http_scheme_e::HTTPS:
        //     return getHTTPS(header, parameter, timeout);

        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        switch (header.GetScheme())
        {
        // case http_scheme_e::HTTP:
        //     return postHTTP(header, body, timeout);

        // case http_scheme_e::HTTPS:
        //     return postHTTPS(header, body, timeout);

        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        switch (header.GetScheme())
        {
        // case http_scheme_e::HTTP:
        //     return postHTTP(header, parameter, timeout);
            
        // case http_scheme_e::HTTPS:
        //     return postHTTPS(header, parameter, timeout);
            
        default:
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, std::string* response)
    {
        ASSERT((response != nullptr), "INPUT PARAMETER <std::string* response> CANNOT BE NULL");

        if (mFlags.test(static_cast<uint8_t>(flag_e::FLASH)) == true)
        {
            File file = esp32FS.Open(mResponsePath);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            
            try
            {
                response->reserve(file.size());
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE DUE TO MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE RESPONSE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            
            while (file.available() > 0)
            {
                *response += file.read();
            }
            file.close();
            ASSERT((file == false), "FILE MUST BE CLOSED TO BE REMOVED");

            for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
            {
                if (esp32FS.Remove(mResponsePath) == Status::Code::GOOD)
                {
                    return Status(Status::Code::GOOD);
                }
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            LOG_ERROR(logger, "FAILED TO REMOVE RESPONSE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else
        {
            ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::CLIENT)) == true), "FLAG MUST BE SET TO CLIENT");

            try
            {
                response->reserve(mContentLength);
                if (mFlags.test(static_cast<uint8_t>(flag_e::HTTP)) == true)
                {
                    while (mClient.available() > 0)
                    {
                        *response += mClient.read();
                    }
                }
                else
                {
                    ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::HTTPS)) == true), "FLAG MUST BE SET TO HTTPS");

                    while (mClientSecure.available() > 0)
                    {
                        *response += mClientSecure.read();
                    }
                }
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE DUE TO MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE RESPONSE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }

            return Status(Status::Code::GOOD);
        }
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, const size_t length, uint8_t output[])
    {
        ASSERT((output != nullptr), "INPUT PARAMETER <uint8_t output[]> CANNOT BE NULL");

        if (mFlags.test(static_cast<uint8_t>(flag_e::FLASH)) == true)
        {
            File file = esp32FS.Open(mResponsePath);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }

            size_t idx = 0;
            while (file.available() > 0)
            {
                output[idx++] = file.read();
                if ((idx + 1) == length)
                {
                    return Status(Status::Code::BAD_OUT_OF_MEMORY);
                }
            }
            file.close();
            ASSERT((file == false), "FILE MUST BE CLOSED TO BE REMOVED");

            return Status(Status::Code::GOOD);
        }
        else
        {
            ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::CLIENT)) == true), "FLAG MUST BE SET TO CLIENT");

            if (mFlags.test(static_cast<uint8_t>(flag_e::HTTP)) == true)
            {
                size_t idx = 0;
                while (mClient.available() > 0)
                {
                    output[idx++] = mClient.read();
                    if ((idx + 1) == length)
                    {
                        return Status(Status::Code::BAD_OUT_OF_MEMORY);
                    }
                }
            }
            else
            {
                ASSERT((mFlags.test(static_cast<uint8_t>(flag_e::HTTPS)) == true), "FLAG MUST BE SET TO HTTPS");

                size_t idx = 0;
                while (mClientSecure.available() > 0)
                {
                    output[idx++] = mClientSecure.read();
                    if ((idx + 1) == length)
                    {
                        return Status(Status::Code::BAD_OUT_OF_MEMORY);
                    }
                }
            }
            
            return Status(Status::Code::GOOD);
        }
    }

    INetwork* LwipHTTP::RetrieveNIC()
    {
        return static_cast<INetwork*>(ethernet);
    }

    int32_t LwipHTTP::RetrieveContentLength() const
    {
        ASSERT(
            (
                (mFlags.test(static_cast<uint8_t>(flag_e::CLIENT)) == true) ||
                (mFlags.test(static_cast<uint8_t>(flag_e::FLASH))  == true)
            ), "INVALID RESPONSE STORAGE FLAG"
        );

        if (mFlags.test(static_cast<uint8_t>(flag_e::FLASH)) == true)
        {
            File file = esp32FS.Open(mResponsePath);
            if (file == false)
            {
                return -1;
            }
            return file.size();
        }
        else
        {
            return mContentLength;
        }
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

        mRSC = 0;
        mContentLength = 0;

        ret = processResponseHeader(timeout);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID RESPONSE HEADER: %s", ret.c_str());
            return ret;
        }

        if ((mRSC % 200) < 100)
        {
            LOG_INFO(logger, "[GET] rsc: %u, length: %d", mRSC, mContentLength);

            if (mContentLength == -1)
            {
                File file = esp32FS.Open(mResponsePath, "w");
                if (file == false)
                {
                    LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                    ret = Status::Code::BAD_DEVICE_FAILURE;
                    goto TEARDOWN;
                }

                while (mClient.available() > 0)
                {
                    file.write(mClient.read());
                }
                file.flush();
                file.close();
                file = esp32FS.Open(mResponsePath, "r");
                if (file == false)
                {
                    LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                    ret = Status::Code::BAD_DEVICE_FAILURE;
                    esp32FS.Remove(mResponsePath);
                    goto TEARDOWN;
                }

                mContentLength = file.size();
                file.close();
            }
            
            ret = Status::Code::GOOD;
            goto TEARDOWN;
        }
        else
        {
            if ((mRSC % 100) < 100)
            {
                LOG_ERROR(logger, "RECEIVED INFORMATIONAL RESPONSE: %u", mRSC);
                ret = Status::Code::BAD_UNKNOWN_RESPONSE;
                goto TEARDOWN;
            }
            else if ((mRSC % 300) < 100)
            {
                LOG_ERROR(logger, "RECEIVED REDIRECTION RESPONSE: %u", mRSC);
                ret = Status::Code::BAD_UNKNOWN_RESPONSE;
                goto TEARDOWN;
            }
            else if ((mRSC % 400) < 100)
            {
                LOG_ERROR(logger, "RECEIVED CLIENT ERROR RESPONSE: %u", mRSC);
                ret = Status::Code::BAD;
                goto TEARDOWN;
            }
            else if ((mRSC % 500) < 100)
            {
                LOG_ERROR(logger, "RECEIVED SERVER ERROR RESPONSE: %u", mRSC);
                ret = Status::Code::BAD;
                goto TEARDOWN;
            }
        }

    TEARDOWN:
        mClient.stop();
        return ret;
    }

/*
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
*/

    Status LwipHTTP::processResponseHeader(const uint16_t timeout)
    {
        mRSC = 0;
        mContentLength = -1;

        const uint32_t startedMillis = millis();
        const uint8_t size = 128;
        char line[size] = {0};
        uint8_t idx = 0;

        while ((millis() - startedMillis) < (timeout * SECOND_IN_MILLIS))
        {
            while (mClient.available() > 0)
            {
                const int value = mClient.read();
                if (value < 0)
                {
                    LOG_ERROR(logger, "FAILED TO READ RxD");
                    return Status(Status::Code::BAD_DATA_LOST);
                }
                
                if (value == '\n')
                {
                    if (strlen(line) == 0)
                    {
                        goto END_OF_HEADER;
                    }
                    
                    line[idx] = '\0';
                    break;
                }
                else if (value == '\r')
                {
                    if (idx == 0)
                    {
                        line[idx] = '\0';
                    }
                }
                else
                {
                    line[idx++] = value;
                }
            }

            if (strncmp(line, "HTTP/", 5) == 0)
            {
                char* pos = strchr(line, ' ');
                mRSC = atoi(++pos);
            }
            else if (strncmp(line, "Content-Length:", 15) == 0)
            {
                char* pos = strchr(line, ' ');
                mContentLength = atoi(++pos);
            }
            idx = 0;
        }

    END_OF_HEADER:
        return Status(Status::Code::GOOD);
    }

/*
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
*/


    LwipHTTP* lwipHTTP = nullptr;
}}