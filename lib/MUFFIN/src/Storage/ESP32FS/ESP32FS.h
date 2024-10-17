/**
 * @file ESP32FS.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ESP32 LittleFS 파일 시스템을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-09-16
 * @version 0.0.1
 * 
 * @note Arduino 프레임워크의 제약으로 인해 End() 실행 결과를 확인할 수 있는 방법이 없음
 * @todo ESP-IDF 프레임워크로 이전 시 파티션 unregister, unmount 결과 값에 따라 실행 결과를 리턴하게 수정해야 함
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Storage/IStorage.h"



namespace muffin {

    class ESP32FS : public IStorage
    {
    public:
        ESP32FS(ESP32FS const&) = delete;
        void operator=(ESP32FS const&) = delete;
        static ESP32FS* GetInstanceOrNULL();
    private:
        ESP32FS();
        virtual ~ESP32FS() override;
    private:
        static ESP32FS* mInstance;
    
    public:
        virtual Status Begin(const bool formatOnFail = false, const char* basePath = "/littlefs", const uint8_t maxOpenFiles = 10, const char* partitionLabel = "spiffs") override;
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
    };
}