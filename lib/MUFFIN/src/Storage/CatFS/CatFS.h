/**
 * @file CatFS.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 파일 시스템을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-11-04
 * @version 1.0.0
 * 
 * @note 현재 Ver.0.0.1에서는 OTA에 필요한 최소한의 기능만을 구현합니다.
 * @todo IStorage 인터페이스를 구현해야 합니다.
 * @todo 함수 시그니처 및 내부 구현을 개선해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Network/CatM1/CatM1.h"
#include "Storage/IStorage.h"



namespace muffin {

    class CatFS : public IStorage
    {
    public:
        CatFS(CatFS const&) = delete;
        void operator=(CatFS const&) = delete;
        static CatFS* CreateInstanceOrNULL(CatM1& catM1);
        static CatFS& GetInstance();
    private:
        CatFS(CatM1& catM1);
        virtual ~CatFS() override;
    private:
        CatM1& mCatM1;
        static CatFS* mInstance;

    public:
        Status BeginWithMutex(size_t mutexHandle, const bool formatOnFail = false, const char* basePath = "/catfs", const uint8_t maxOpenFiles = 1, const char* partitionLabel = "catfs");
        virtual Status Begin(const bool formatOnFail = false, const char* basePath = "/catfs", const uint8_t maxOpenFiles = 1, const char* partitionLabel = "catfs") override;
        virtual Status Format() override;
        virtual size_t GetTotalBytes() const override;
        virtual size_t GetUsedBytes() const override;
        virtual Status End() override;
    public:
        virtual File Open(const char* path, const char* mode = FILE_READ, const bool create = false) override;
        virtual File Open(const std::string& path, const char* mode = FILE_READ, const bool create = false) override;
        virtual Status DoesExist(const char* path) override;
        virtual Status DoesExist(const std::string& path) override;
        virtual Status Remove(const char* path) override;
        virtual Status Remove(const std::string& path) override;
        virtual Status Rename(const char* pathFrom, const char* pathTo) override;
        virtual Status Rename(const std::string& pathFrom, const std::string& pathTo) override;
        virtual Status MakeDirectory(const char* path) override;
        virtual Status MakeDirectory(const std::string &path) override;
        virtual Status RemoveDirectory(const char* path) override;
        virtual Status RemoveDirectory(const std::string &path) override;
    public:
        Status Open(const std::string& path, bool temporary = true);
        Status OpenWithMutex(size_t mutexHandle, const std::string& path, const bool temporary = true);
        Status Close();
        Status CloseWithMutex(size_t mutexHandle);
        Status Read(const size_t length, uint8_t outputBuffer[]);
        Status ReadWithMutex(size_t mutexHandle, const size_t length, uint8_t outputBuffer[]);
        Status Seek(const size_t offset);
        Status DownloadFile(const std::string& path, std::string* data);
    private:
	    uint16_t calculateChecksum(const std::string contents);
    private:
        Status readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd);
        std::pair<Status, size_t> processMetadataForQFREAD(const uint32_t timeoutMillis);
        Status readBytes(const uint32_t timeoutMillis, const size_t length, uint8_t outputBuffer[]);
        Status processCmeErrorCode(const std::string& rxd);
    private:
        int32_t mFileHandle = -1;
    };
}