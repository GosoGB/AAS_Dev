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
            return mutexHandle.first;
        }
        
        const std::string command = "AT+QFLDS";
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
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        mCatM1.ReleaseMutex();
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

    Status CatFS::Open(const std::string& path)
    {
        ASSERT((path.length() < 81), "PATH MUST BE LESS THAN OR EQUAL TO 80 CHARACTERS");
        ASSERT((mFileHandle == -1), "INVALID FILE HANDLE: %d", mFileHandle);
        
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
        if (ret == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s, %s", ret.c_str(), rxd.c_str());
            return ret;
        }

        const size_t pos1 = rxd.find("+QFOPEN: ");
        const size_t pos2 = rxd.find("\r\n", pos1 + 1);
        const std::string strFileHandle = rxd.substr(pos1, (pos2 - pos1));
        LOG_DEBUG(logger, "File Handle: %s", strFileHandle.c_str());
        mFileHandle = Convert.ToInt32(strFileHandle);

        if (mFileHandle < 0)
        {
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
            LOG_ERROR(logger, "FAILED TO CLOSE: %s, %s", ret.c_str(), rxd.c_str());
        }
        
        mFileHandle = -1;
        return ret;
    }

    Status CatFS::Read(const size_t length, uint8_t outputBuffer[])
    {
        ASSERT((mFileHandle != -1), "INVALID FILE HANDLE: %d", mFileHandle);
        
        constexpr uint8_t BUFFER_SIZE = 128;

        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFREAD=%u,%u", mFileHandle, length);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 1000;
        std::string rxd;

        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s", ret.c_str());
            mCatM1.ReleaseMutex();
            return ret;
        }
        
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        mCatM1.ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN: %s, %s", ret.c_str(), rxd.c_str());
            return ret;
        }

        const size_t pos1 = rxd.find("+QFOPEN: ");
        const size_t pos2 = rxd.find("\r\n", pos1 + 1);
        const std::string strFileHandle = rxd.substr(pos1, (pos2 - pos1));
        LOG_DEBUG(logger, "File Handle: %s", strFileHandle.c_str());
        mFileHandle = Convert.ToInt32(strFileHandle);

        if (mFileHandle < 0)
        {
            mFileHandle = -1;
            return Status(Status::Code::BAD);
        }
        
        return ret;
    }

    Status CatFS::Seek(const size_t offset)
    {
        ASSERT((mFileHandle != -1), "INVALID FILE HANDLE: %d", mFileHandle);

        constexpr uint8_t BUFFER_SIZE = 64;

        char command[BUFFER_SIZE] = { 0 };
        sprintf(command, "AT+QFSEEK=%u,%llu,0", mFileHandle, offset);
        ASSERT((strlen(command) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        const auto mutexHandle = mCatM1.TakeMutex();
        if (mutexHandle.first.ToCode() != Status::Code::GOOD)
        {
            return mutexHandle.first;
        }

        Status ret = mCatM1.Execute(command, mutexHandle.second);
        if (ret == Status::Code::GOOD)
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
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }


    CatFS* CatFS::mInstance = nullptr;
}