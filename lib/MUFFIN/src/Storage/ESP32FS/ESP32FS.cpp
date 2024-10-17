/**
 * @file ESP32FS.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ESP32 LittleFS 파일 시스템을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-09-16
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <LittleFS.h>

#include "ESP32FS.h"
#include "Common/Logger/Logger.h"



namespace muffin {

    ESP32FS* ESP32FS::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) ESP32FS();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR ESP32 FS");
                return nullptr;
            }
        }
        
        return mInstance;
    }

    ESP32FS::ESP32FS()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    #endif
    }

    ESP32FS::~ESP32FS()
    {
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status ESP32FS::Begin(const bool formatOnFail, const char* basePath, const uint8_t maxOpenFiles, const char* partitionLabel)
    {
        if (LittleFS.begin(formatOnFail, basePath, maxOpenFiles, partitionLabel) == true)
        {
            LOG_INFO(logger, "Succedded to begin ESP32 file system");
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO START ESP32 FILE SYSTEM");
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::Format()
    {
        if (LittleFS.format() == true)
        {
            LOG_INFO(logger, "Succedded to format ESP32 file system");
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO FORMAT ESP32 FILE SYSTEM");
            return Status(Status::Code::BAD);
        }
    }

    size_t ESP32FS::GetTotalBytes() const
    {
        return LittleFS.totalBytes();
    }

    size_t ESP32FS::GetUsedBytes() const
    {
        return LittleFS.usedBytes();
    }

    Status ESP32FS::End()
    {
        LOG_INFO(logger, "Trying to end the ESP32 file system");
        LittleFS.end();

        return Status(Status::Code::UNCERTAIN);
    }

    File ESP32FS::Open(const char* path, const char* mode, const bool create)
    {
        LOG_DEBUG(logger, "Path: %s, Mode: %s, Create: %s", path, mode, create ? "true" : "false");
        return LittleFS.open(path, mode, create);
    }

    File ESP32FS::Open(const std::string& path, const char* mode, const bool create)
    {
        return Open(path.c_str(), mode, create);
    }

    Status ESP32FS::DoesExist(const char* path)
    {
        if (LittleFS.exists(path) == true)
        {
            LOG_DEBUG(logger, "Found: %s", path);
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_DEBUG(logger, "Not found: %s", path);
            return Status(Status::Code::BAD_NOT_FOUND);
        }
    }

    Status ESP32FS::DoesExist(const std::string& path)
    {
        return DoesExist(path.c_str());
    }

    Status ESP32FS::Remove(const char* path)
    {
        if (LittleFS.remove(path) == true)
        {
            LOG_DEBUG(logger, "Removed: %s", path);
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO REMOVE FILE: %s", path);
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::Remove(const std::string& path)
    {
        return Remove(path.c_str());
    }

    Status ESP32FS::Rename(const char* pathFrom, const char* pathTo)
    {
        if (LittleFS.rename(pathFrom, pathTo) == true)
        {
            LOG_DEBUG(logger, "Renamed: %s to %s", pathFrom, pathTo);
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO RENAME %s TO %s", pathFrom, pathTo);
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::Rename(const std::string& pathFrom, const std::string& pathTo)
    {
        return Rename(pathFrom.c_str(), pathTo.c_str());
    }

    Status ESP32FS::MakeDirectory(const char* path)
    {
        if (LittleFS.mkdir(path) == true)
        {
            LOG_DEBUG(logger, "Directory created: %s", path);
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO MAKE DIRECTORY: %s", path);
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::MakeDirectory(const std::string &path)
    {
        return MakeDirectory(path.c_str());
    }

    Status ESP32FS::RemoveDirectory(const char* path)
    {
        if (LittleFS.rmdir(path) == true)
        {
            LOG_DEBUG(logger, "Directory removed: %s", path);
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO REMOVE DIRECTORY: %s", path);
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::RemoveDirectory(const std::string &path)
    {
        return RemoveDirectory(path.c_str());
    }


    ESP32FS* ESP32FS::mInstance = nullptr;
}