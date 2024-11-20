/**
 * @file IStorage.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 모든 저장 장치의 파일 시스템이 공통으로 따르는 소프트웨어 인터페이스를 선언합니다.
 * 
 * @date 2024-09-05
 * @version 1.0.0
 * 
 * @todo CatFS 드라이버를 작성하여 VFS에 연결하는 작업
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <FS.h>
#include <string>

#include "Common/Status.h"



namespace muffin {
    
    class IStorage
    {
    public:
        IStorage() {}
        virtual ~IStorage() {}
    public:
        virtual Status Begin(const bool formatOnFail, const char* basePath, const uint8_t maxOpenFiles, const char* partitionLabel) = 0;
        virtual Status Format() = 0;
        virtual size_t GetTotalBytes() const = 0;
        virtual size_t GetUsedBytes() const= 0;
        virtual Status End() = 0;
    public:
        virtual File Open(const char* path, const char* mode = FILE_READ, const bool create = false) = 0;
        virtual File Open(const std::string& path, const char* mode = FILE_READ, const bool create = false) = 0;
        virtual Status DoesExist(const char* path) = 0;
        virtual Status DoesExist(const std::string& path) = 0;
        virtual Status Remove(const char* path) = 0;
        virtual Status Remove(const std::string& path) = 0;
        virtual Status Rename(const char* pathFrom, const char* pathTo) = 0;
        virtual Status Rename(const std::string& pathFrom, const std::string& pathTo) = 0;
        virtual Status MakeDirectory(const char* path) = 0;
        virtual Status MakeDirectory(const std::string &path) = 0;
        virtual Status RemoveDirectory(const char* path) = 0;
        virtual Status RemoveDirectory(const std::string &path) = 0;
    };
}