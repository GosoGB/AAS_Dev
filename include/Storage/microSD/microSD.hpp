/**
 * @file microSD.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-11
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>



namespace muffin { 


    class microSD
    {
    public:
        microSD(microSD const&) = delete;
        void operator=(microSD const&) = delete;
        static microSD* GetInstance();
    private:
        microSD() = default;
        ~microSD() noexcept = default;
    private:
        static microSD mInstance;

    public:
        // Status Begin(const bool formatOnFail = false, const char* basePath = "/littlefs", const uint8_t maxOpenFiles = 10, const char* partitionLabel = "spiffs") override;
        // Status Format() override;
        // size_t GetTotalBytes() const override;
        // size_t GetUsedBytes() const override;
        // Status End() override;
    };
}