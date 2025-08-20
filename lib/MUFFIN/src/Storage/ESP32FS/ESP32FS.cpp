/**
 * @file ESP32FS.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief ESP32 LittleFS 파일 시스템을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <esp_littlefs.h>
#include <LittleFS.h>
#include <nvs_flash.h>
#include <Preferences.h>

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    ESP32FS::ESP32FS()
    {
        esp_err_t ret = nvs_flash_init();
        if (ret != ESP_OK)
        {
            std::cerr << "\n\n\033[31mFAILED TO INITIALIZE NVS PARTITION: " << ret << std::endl;
            std::abort();
        }
        
        Preferences pf;
        const char* key = "firstBoot";

        if (pf.begin(NVS_NAMESPACE_INIT, false) == false)
        {
            std::cerr << "\n\n\033[31mFAILED TO BEGIN NVS PARTITION" << std::endl;
            std::abort();
        }

        if (pf.isKey(key) == false)
        {
            if (LittleFS.begin(false) == true)
            {
                if (LittleFS.exists(JARVIS_PATH) == true || LittleFS.exists(DEPRECATED_JARVIS_PATH) == true)
                {
                    pf.putBool(key, false);
                    if (pf.getBool(key, true) == true)
                    {
                        std::cerr << "\n\n\033[31mFAILED TO UPDATE FIRST SYSTEM BOOT INFO" << std::endl;
                        pf.clear();
                        std::abort();
                    }

                    std::cout << "\033[34mFirst System Boot Detected But JARVIS File already exists." << std::endl;
                    return;
                }
            }

            std::cout << "\033[34mFirst System Boot Detected. Formatting flash memory" << std::endl;

            const char* partitionLabel = "spiffs";
            ret = esp_littlefs_format(partitionLabel);
            if (ret != ESP_OK)
            {
                std::cerr << "\n\n\033[31mFAILED TO FORMAT THE ESP32 FS" << std::endl;
                std::abort();
            }

            pf.putBool(key, false);
            if (pf.getBool(key, true) == true)
            {
                std::cerr << "\n\n\033[31mFAILED TO UPDATE FIRST SYSTEM BOOT INFO" << std::endl;
                pf.clear();
                std::abort();
            }
            std::cout << "\033[34mESP32 FS has been formatted \n\n \033[0m" << std::endl;
            vTaskDelay(100);
        }

    }

    Status ESP32FS::Begin(const bool formatOnFail, const char* basePath, const uint8_t maxOpenFiles, const char* partitionLabel)
    {
        if (LittleFS.begin(formatOnFail, basePath, maxOpenFiles, partitionLabel) == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD);
        }
    }

    Status ESP32FS::Format()
    {
        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            if (LittleFS.format() == true)
            {
                return Status(Status::Code::GOOD);
            }
            else
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

        return Status(Status::Code::BAD_DEVICE_FAILURE);
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
        LOG_VERBOSE(logger, "Path: %s, Mode: %s, Create: %s", path, mode, create ? "true" : "false");
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
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_NOT_FOUND);
        }
    }

    Status ESP32FS::DoesExist(const std::string& path)
    {
        return DoesExist(path.c_str());
    }

    Status ESP32FS::Remove(const char* path)
    {
        if (DoesExist(path) == Status::Code::BAD_NOT_FOUND)
        {
            return Status(Status::Code::GOOD);
        }
        
        for (uint8_t trialCount = 0; trialCount < MAX_RETRY_COUNT; ++trialCount)
        {
            if (LittleFS.remove(path) == true)
            {
                return Status(Status::Code::GOOD);
            }
            else
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        
        return Status(Status::Code::BAD_DEVICE_FAILURE);
    }

    Status ESP32FS::Remove(const std::string& path)
    {
        return Remove(path.c_str());
    }

    Status ESP32FS::Rename(const char* pathFrom, const char* pathTo)
    {
        if (LittleFS.rename(pathFrom, pathTo) == true)
        {
            LOG_VERBOSE(logger, "Renamed: %s to %s", pathFrom, pathTo);
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
            LOG_VERBOSE(logger, "Directory created: %s", path);
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
            LOG_VERBOSE(logger, "Directory removed: %s", path);
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


    ESP32FS esp32FS;
}