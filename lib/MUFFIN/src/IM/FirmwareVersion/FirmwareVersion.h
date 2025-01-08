/**
 * @file FirmwareVersion.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-12-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#pragma once

#include <string.h>
#include <sys/_stdint.h>



namespace muffin {

    class FirmwareVersion
    {
    public:
        FirmwareVersion(const char* semver, const uint32_t code);
        virtual ~FirmwareVersion() {}
    public:
        void SetSemanticVersion(const char* semver);
        void SetVersionCode(const uint32_t code);
    public:
        const char* GetSemanticVersion() const;
        uint32_t GetVersionCode() const;
    private:
        char mSemanticVersion[16];
        uint32_t mVersionCode;
    };


    extern FirmwareVersion FW_VERSION_ESP32;
#if defined(MODLINK_T2)
    extern FirmwareVersion FW_VERSION_MEGA2560;
#endif
}