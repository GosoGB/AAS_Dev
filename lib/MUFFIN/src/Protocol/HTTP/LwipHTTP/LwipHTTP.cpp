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
    #if defined(DEBUG)
        mClientSecure.setCACert(ROOT_CA_CRT_DEV);
    #else
        mClientSecure.setCACert(ROOT_CA_CRT);
    #endif
        mFlags.reset();
        return Status(Status::Code::GOOD);
    }
    
    Status LwipHTTP::GET(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        mFlags.reset(static_cast<uint8_t>(flag_e::RSC_OK));
        mFlags.reset(static_cast<uint8_t>(flag_e::OCTET));
        Status ret(Status::Code::UNCERTAIN);
        mFilePosition = 0;

        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTP));
            ret = getHTTP(header, parameter, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTP));
            break;

        case http_scheme_e::HTTPS:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTPS));
            ret = getHTTPS(header, parameter, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTPS));
            break;

        default:
            ret = Status(Status::Code::BAD_INVALID_ARGUMENT);
            break;
        }

        if (ret == Status::Code::GOOD)
        {
            mFlags.set(static_cast<uint8_t>(flag_e::RSC_OK));
        }

        return ret;
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        mFlags.reset(static_cast<uint8_t>(flag_e::RSC_OK));
        mFlags.reset(static_cast<uint8_t>(flag_e::OCTET));
        Status ret(Status::Code::UNCERTAIN);
        mFilePosition = 0;

        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTP));
            ret = postHTTP(header, body, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTP));
            break;

        case http_scheme_e::HTTPS:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTPS));
            ret = postHTTPS(header, body, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTPS));
            break;

        default:
            ret = Status(Status::Code::BAD_INVALID_ARGUMENT);
            break;
        }

        if (ret == Status::Code::GOOD)
        {
            mFlags.set(static_cast<uint8_t>(flag_e::RSC_OK));
        }

        return ret;
    }

    Status LwipHTTP::POST(const size_t mutexHandle, RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        mFlags.reset(static_cast<uint8_t>(flag_e::RSC_OK));
        mFlags.reset(static_cast<uint8_t>(flag_e::OCTET));
        Status ret(Status::Code::UNCERTAIN);
        mFilePosition = 0;

        switch (header.GetScheme())
        {
        case http_scheme_e::HTTP:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTP));
            ret = postHTTP(header, parameter, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTP));
            break;

        case http_scheme_e::HTTPS:
            mFlags.set(static_cast<uint8_t>(flag_e::HTTPS));
            ret = postHTTPS(header, parameter, timeout);
            mFlags.reset(static_cast<uint8_t>(flag_e::HTTPS));
            break;

        default:
            ret = Status(Status::Code::BAD_INVALID_ARGUMENT);
            break;
        }

        if (ret == Status::Code::GOOD)
        {
            mFlags.set(static_cast<uint8_t>(flag_e::RSC_OK));
        }

        return ret;
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, std::string* response)
    {
        ASSERT((response != nullptr), "INPUT PARAMETER <std::string* response> CANNOT BE NULL");

        if (mFlags.test(static_cast<uint8_t>(flag_e::RSC_OK)) == false)
        {
            return Status(Status::Code::BAD_INVALID_STATE);
        }

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
        esp32FS.Remove(mResponsePath);

        return Status(Status::Code::GOOD);
    }

    Status LwipHTTP::Retrieve(const size_t mutexHandle, const size_t length, uint8_t output[])
    {
        ASSERT((length > 0), "INPUT PARAMETER <const size_t length> MUST BE GREATER THAN 0");
        ASSERT((output != nullptr), "INPUT PARAMETER <uint8_t output[]> CANNOT BE NULL");

        if (mFlags.test(static_cast<uint8_t>(flag_e::RSC_OK)) == false)
        {
            return Status(Status::Code::BAD_INVALID_STATE);
        }

        File file = esp32FS.Open(mResponsePath);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        if (mFilePosition > 0)
        {
            LOG_DEBUG(logger, "File position: %u", mFilePosition);
            const bool isSought = file.seek(++mFilePosition);
            if (isSought == false)
            {
                file.close();
                return Status(Status::Code::BAD_OUT_OF_RANGE);
            }
        }
        
        memset(output, 0, length);
        file.readBytes(reinterpret_cast<char*>(output), length);
        mFilePosition = file.position();
        file.close();
        ASSERT((file == false), "FILE MUST BE CLOSED TO BE REMOVED");

        return Status(Status::Code::GOOD);
    }

    int32_t LwipHTTP::RetrieveContentLength() const
    {
        File file = esp32FS.Open(mResponsePath);
        if (file == false)
        {
            return -1;
        }
        const int32_t size = file.size();
        file.close();
        return size;
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
        ret = processResponseHeader(timeout);
        LOG_INFO(logger, "[GET] rsc: %u", mRSC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID RESPONSE HEADER: %s", ret.c_str());
            return ret;
        }

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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

    Status LwipHTTP::getHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        if (mClientSecure.connect(header.GetHost().c_str(), header.GetPort()) == false)
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

        mClientSecure.print(header.ToString().c_str());
        mClientSecure.flush();

        mRSC = 0;
        mContentLength = 0;

        ret = processResponseHeader(timeout);
        LOG_INFO(logger, "[GET] rsc: %u", mRSC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID RESPONSE HEADER: %s", ret.c_str());
            return ret;
        }

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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
        mClientSecure.stop();
        return ret;
    }

    Status LwipHTTP::postHTTP(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
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
        ret = processResponseHeader(timeout);
        LOG_INFO(logger, "[POST] rsc: %u", mRSC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID RESPONSE HEADER: %s", ret.c_str());
            return ret;
        }

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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

    Status LwipHTTP::postHTTP(RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        if (mClient.connect(header.GetHost().c_str(), header.GetPort()) == false)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT: %s:%u", header.GetHost().c_str(), header.GetPort());
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }

        Status ret = header.SetContentLength(strlen(body.ToString().c_str()));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET CONTENT LENGTH: %s", body.ToString().c_str());
            return ret;
        }
            
        ret = header.SetContentType(body.GetContentType());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET CONTENT TYPE: %s", body.GetContentType());
            return ret;
        }
        
        mClient.print(header.ToString().c_str());
        mClient.print(body.ToString().c_str());

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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

    Status LwipHTTP::postHTTPS(RequestHeader& header, const RequestParameter& parameter, const uint16_t timeout)
    {
        if (mClientSecure.connect(header.GetHost().c_str(), header.GetPort()) == false)
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
        
        mClientSecure.print(header.ToString().c_str());

        mRSC = 0;
        ret = processResponseHeader(timeout);
        LOG_INFO(logger, "[POST] rsc: %u", mRSC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID RESPONSE HEADER: %s", ret.c_str());
            return ret;
        }

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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
        mClientSecure.stop();
        return ret;
    }

    Status LwipHTTP::postHTTPS(RequestHeader& header, const RequestBody& body, const uint16_t timeout)
    {
        if (mClientSecure.connect(header.GetHost().c_str(), header.GetPort()) == false)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT: %s:%u", header.GetHost().c_str(), header.GetPort());
            return Status(Status::Code::BAD_SERVER_NOT_CONNECTED);
        }

        Status ret = header.SetContentLength(strlen(body.ToString().c_str()));
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET CONTENT LENGTH: %s", body.ToString().c_str());
            return ret;
        }
            
        ret = header.SetContentType(body.GetContentType());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET CONTENT TYPE: %s", body.GetContentType());
            return ret;
        }
        
        mClientSecure.print(header.ToString().c_str());
        mClientSecure.print(body.ToString().c_str());

        if ((mRSC % 200) < 100)
        {
            ret = saveResponseBody();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO SAVE RESPONSE BODY: %s", ret.c_str());
                return ret;
            }

            File file = esp32FS.Open(mResponsePath, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
                ret = Status::Code::BAD_DEVICE_FAILURE;
                esp32FS.Remove(mResponsePath);
                goto TEARDOWN;
            }

            mContentLength = file.size();
            file.close();
            
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
        mClientSecure.stop();
        return ret;
    }

    Status LwipHTTP::processResponseHeader(const uint16_t timeout)
    {
        mRSC = 0;
        mContentLength = -1;

        const uint32_t startedMillis = millis();
        const uint8_t size = 128;
        char line[size] = {0};
        uint8_t idx = 0;

        WiFiClient* client = mFlags.test(static_cast<uint8_t>(flag_e::HTTP)) ? &mClient : &mClientSecure;

        while ((millis() - startedMillis) < (timeout * SECOND_IN_MILLIS))
        {
            while (client->available() > 0)
            {
                const int value = client->read();
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
            else if (strncmp(line, "Content-Type: application/octet-stream", 38) == 0)
            {
                mFlags.set(static_cast<uint8_t>(flag_e::OCTET));
            }
            
            idx = 0;
        }

        if (mRSC == 0)
        {
            return Status(Status::Code::BAD_NO_DATA);
        }

    END_OF_HEADER:
        return Status(Status::Code::GOOD);
    }

    Status LwipHTTP::saveResponseBody()
    {
        WiFiClient* client = mFlags.test(static_cast<uint8_t>(flag_e::HTTP)) ? &mClient : &mClientSecure;
        Status ret(Status::Code::UNCERTAIN);

        File file = esp32FS.Open(mResponsePath, "w", true);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN RESPONSE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        if (mFlags.test(static_cast<uint8_t>(flag_e::OCTET)) == false)
        {
            while (client->available() > 0)
            {
                if (client->peek() == '\n')
                {
                    client->read();
                    break;
                }
                
                client->read();
            }
        }

        while (client->available() > 0)
        {
            if (mFlags.test(static_cast<uint8_t>(flag_e::OCTET)) == false)
            {
                if ((client->available() == 1) && (client->peek() == '0'))
                {
                    client->read();
                    file.write(0);
                    break;
                }
            }

            file.write(client->read());
        }
        file.flush();
        file.close();

        return Status(Status::Code::GOOD);
    }


    LwipHTTP* lwipHTTP = nullptr;
}}