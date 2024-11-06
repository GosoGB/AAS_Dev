/**
 * @file CatFS.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 파일 시스템을 사용하는데 필요한 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-11-04
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <sstream>

#include "CatFS.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"



namespace muffin {

    CatFS* CatFS::CreateInstanceOrNULL(CatM1& catM1)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) CatFS(catM1);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO ALLOCATE MEMORY FOR Cat.M1 FILE SYSTEM");
                return mInstance;
            }
        }
        
        return mInstance;
    }

    CatFS& CatFS::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    CatFS::CatFS(CatM1& catM1)
        : mCatM1(catM1)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    CatFS::~CatFS()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status CatFS::Begin(const bool formatOnFail, const char* basePath, const uint8_t maxOpenFiles, const char* partitionLabel)
    {
        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
            return mutexHandle.first;
        }
        
        const std::string command = "AT+QFLDS=\"UFS\"";
        const uint32_t timeoutMillis = 500;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            mCatM1.ReleaseMutex();
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        const std::string prefix = "+QFLDS: ";
        const std::string postfix = "\r\n";

        size_t start = rxd.find(prefix);
        size_t end   = rxd.find(postfix, (start + 1));
        if (start == std::string::npos || end == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        start += prefix.length();

        const std::string response = rxd.substr(start, (end - start));
        std::istringstream iss(response);
        std::string strFreeBytes;
        std::string strTotalBytes;
        if (!std::getline(iss, strFreeBytes, ',') || !std::getline(iss, strTotalBytes))
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const size_t freeBytes  = Convert.ToUInt32(strFreeBytes);
        const size_t totalBytes = Convert.ToUInt32(strTotalBytes);
        LOG_INFO(logger, "Free Space: %u, Total Space: %u", freeBytes, totalBytes);
        return ret;
    }
    
    Status CatFS::Format()
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
    
    size_t CatFS::GetTotalBytes() const
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return 0;
    }
    
    size_t CatFS::GetUsedBytes() const
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return SIZE_MAX;
    }
    
    Status CatFS::End()
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    File CatFS::Open(const char* path, const char* mode, const bool create)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return File();
    }

    File CatFS::Open(const std::string& path, const char* mode, const bool create)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return File();
    }

    Status CatFS::DoesExist(const char* path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::DoesExist(const std::string& path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::Remove(const char* path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::Remove(const std::string& path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::Rename(const char* pathFrom, const char* pathTo)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::Rename(const std::string& pathFrom, const std::string& pathTo)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::MakeDirectory(const char* path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::MakeDirectory(const std::string &path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::RemoveDirectory(const char* path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::RemoveDirectory(const std::string &path)
    {
        ASSERT(false, "UNSUPPORTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CatFS::Open(const std::string& path, bool temporary)
    {
        ASSERT((path.length() < 81), "PATH MUST BE LESS THAN OR EQUAL TO 80 CHARACTERS");
        /**
         * @todo 복수의 파일을 동시에 열 수 있게 코드를 수정해야 합니다.
         */
        ASSERT((mFileHandle == -1), "ALLOWED TO OPEN ONLY ONE FILE AT A TIME");
        if (temporary)
        {
            LOG_WARNING(logger, "THIS FUNCTION WILL BE DEPRECATED");
        }
        
        /**
         * @todo 파일 모드에 따라 파일을 열 수 있게 코드를 수정해야 합니다.
         *       현재는 read-only로만 열 수 있습니다.
         */
        constexpr uint8_t BUFFER_SIZE = 128;
        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFOPEN=\"%s\",2", path.c_str());
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s", ret.c_str());
            return ret;
        }

        
        const std::string prefix = "+QFOPEN: ";
        const std::string postfix = "\r\n";

        size_t start = rxd.find(prefix);
        size_t end   = rxd.find(postfix, (start + 1));
        if (start == std::string::npos || end == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        start += prefix.length();

        const std::string strFileHandle = rxd.substr(start, (end - start));
        LOG_DEBUG(logger, "strFileHandle: %s", strFileHandle.c_str());
        mFileHandle = Convert.ToInt32(strFileHandle);
        if (mFileHandle < 0 || mFileHandle == SIZE_MAX)
        {
            LOG_ERROR(logger, "INVALID FILE HANDLE: %u", mFileHandle);
            mFileHandle = -1;
            return Status(Status::Code::BAD);
        }
        
        return ret;
    }

    Status CatFS::Close()
    {
        ASSERT((mFileHandle != -1), "INVALID FILE HANDLE: %d", mFileHandle);
        
        constexpr uint8_t BUFFER_SIZE = 32;
        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFCLOSE=%u", mFileHandle);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");

        const uint32_t timeoutMillis = 300;
        std::string rxd;

        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE: %s", ret.c_str());
        }
        
        mFileHandle = -1;
        return ret;
    }

    Status CatFS::Read(const size_t length, uint8_t outputBuffer[])
    {
        ASSERT((mFileHandle != -1), "INVALID FILE HANDLE: %d", mFileHandle);
        
        constexpr uint8_t BUFFER_SIZE = 32;
        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFREAD=%u,%u", mFileHandle, length);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        const uint32_t timeoutMillis = 1000;
        const auto retMetadata = processMetadataForQFREAD(timeoutMillis);
        if (retMetadata.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ: %s", retMetadata.first.c_str());
            mCatM1.ReleaseMutex();
            return retMetadata.first;
        }
        else if (retMetadata.second != length)
        {
            LOG_ERROR(logger, "BYTES READ DOES NOT MATCH: Actual: %u, Command: %u", retMetadata.second, length);
            mCatM1.ReleaseMutex();

            if (retMetadata.second < length)
            {
                return Status(Status::Code::BAD_DATA_LOST);
            }
            else
            {
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
        }

        ret = readBytes(timeoutMillis, retMetadata.second, outputBuffer);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ DATA: %s", ret.c_str());
            return ret;
        }
        
        for (size_t i = 0; i < length; ++i)
        {
            Serial.print(outputBuffer[i]);
            Serial.print(" ");
        }
        Serial.println();
        return ret;
    }

    Status CatFS::Seek(const size_t offset)
    {
        ASSERT((mFileHandle != -1), "INVALID FILE HANDLE: %d", mFileHandle);

        constexpr uint8_t BUFFER_SIZE = 64;
        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFSEEK=%u,%u,0", mFileHandle, offset);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SEEK: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SEEK: %s, %s", ret.c_str(), rxd.c_str());
        }
        
        return ret;
    }

    Status CatFS::readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd)
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

            if (rxd->find("OK\r\n") != std::string::npos)
            {
                return Status(Status::Code::GOOD);
            }
            else if (rxd->find("ERROR\r\n") != std::string::npos)
            {
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
            else if (rxd->find("+CME ERROR:") != std::string::npos)
            {
                return processCmeErrorCode(*rxd);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    std::pair<Status, size_t> CatFS::processMetadataForQFREAD(const uint32_t timeoutMillis)
    {
        std::string rxd;
        std::string prefix = "CONNECT ";
        std::string postfix = "\r\n";
        size_t start;
        size_t end;

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
                rxd += value;

                start = rxd.find(prefix);
                end   = rxd.find(postfix, start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    goto TARGET_FOUND;
                }
            }

            if (rxd.find("ERROR\r\n") != std::string::npos)
            {
                return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), SIZE_MAX);
            }
            else if (rxd.find("+CME ERROR:") != std::string::npos)
            {
                return std::make_pair(processCmeErrorCode(rxd), SIZE_MAX);
            }
            else
            {
                continue;
            }

        }
        return std::make_pair(Status(Status::Code::BAD_TIMEOUT), SIZE_MAX);


    TARGET_FOUND:
        std::string response = rxd.substr(start, end);
        while (response.empty() == false && (response.back() == '\r' || response.back() == '\n'))
        {
            response.pop_back();
        }
        
        std::string strLength = response.substr(response.find(" ") + 1);
        const size_t length = Convert.ToUInt32(strLength);
        return std::make_pair(Status(Status::Code::GOOD), length);
    }

    Status CatFS::readBytes(const uint32_t timeoutMillis, const size_t length, uint8_t outputBuffer[])
    {
        std::string rxd;
        std::string result = "\r\nOK\r\n";
        size_t byteCount = 0;

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
                outputBuffer[byteCount++] = value;

                if (byteCount == length)
                {
                    LOG_DEBUG(logger, "Byte Count: %u", byteCount);
                    goto READ_FINISHED;
                }
            }
            continue;
        }
        
        if (rxd.substr(rxd.length() - 18).find("\r\n+CME ERROR: ") != std::string::npos)
        {
            return processCmeErrorCode(rxd);
        }
        else if (rxd.find("\r\nERROR\r\n") != std::string::npos)
        {
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        else
        {
            return Status(Status::Code::BAD_TIMEOUT);
        }
        
    
    READ_FINISHED:
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
                rxd += value;
            }
            LOG_DEBUG(logger, "rxd: %s", rxd.c_str());

            if (rxd.find(result) != std::string::npos)
            {
                return Status(Status::Code::GOOD);
            }
            continue;
        }
        return processCmeErrorCode(rxd);
    }

    Status CatFS::processCmeErrorCode(const std::string& rxd)
    {
        const std::string prefix = "+CME ERROR: ";
        size_t start = rxd.find(prefix);
        size_t end   = rxd.find("\r", start + 1);
        if (start == std::string::npos || end == std::string::npos)
        {
            LOG_ERROR(logger, "INVALID CME ERROR CODE: %s", rxd.c_str());
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        start += prefix.length();

        const std::string strCode = rxd.substr(start, (end - start));
        const uint32_t cmeErrorCode = Convert.ToUInt32(strCode);
        switch (cmeErrorCode)
        {
        case 400:
            LOG_ERROR(logger, "INVALID INPUT VALUE");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        case 401:
            LOG_ERROR(logger, "LARGER THAN THE SIZE OF THE FILE");
            return Status(Status::Code::BAD_DATA_UNAVAILABLE);
        case 402:
            LOG_ERROR(logger, "READ ZERO BYTE");
            return Status(Status::Code::BAD_NO_DATA);
        case 403:
            LOG_ERROR(logger, "DRIVE FULL");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 405:
            LOG_ERROR(logger, "FILE NOT FOUND");
            return Status(Status::Code::BAD_NOT_FOUND);
        case 406:
            LOG_ERROR(logger, "INVALID FILE NAME");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        case 407:
            LOG_ERROR(logger, "FILE ALREADY EXISTS");
            return Status(Status::Code::BAD_ENTRY_EXISTS);
        case 409:
            LOG_ERROR(logger, "FAIL TO WRITE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 410:
            LOG_ERROR(logger, "FAIL TO OPEN FILE OR FILE DOES NOT EXIST");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 411:
            LOG_ERROR(logger, "FAIL TO READ THE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 413:
            LOG_ERROR(logger, "REACHED THE MAX NUMBER OF FILE TO OPEN");
            return Status(Status::Code::BAD_TOO_MANY_OPERATIONS);
        case 414:
            LOG_ERROR(logger, "THE FILE IS READ-ONLY");
            return Status(Status::Code::BAD_NOT_WRITABLE);
        case 416:
            LOG_ERROR(logger, "INVALID FILE DESCRIPTOR");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 417:
            LOG_ERROR(logger, "FAIL TO LIST THE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 418:
            LOG_ERROR(logger, "FAIL TO DELETE THE FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 419:
            LOG_ERROR(logger, "FAIL TO GET STORAGE INFO");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 420:
            LOG_ERROR(logger, "NO SPACE LEFT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 421:
            LOG_ERROR(logger, "TIME OUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 423:
            LOG_ERROR(logger, "FILE TOO LARGE");
            return Status(Status::Code::BAD_REQUEST_TOO_LARGE);
        case 425:
            LOG_ERROR(logger, "INVALID PARAMETER");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        case 426:
            LOG_ERROR(logger, "FILE ALREADY OPENED");
            return Status(Status::Code::BAD_NOTHING_TO_DO);
        default:
            LOG_ERROR(logger, "UNKNOWN CME ERROR CODE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
    }


    CatFS* CatFS::mInstance = nullptr;
}