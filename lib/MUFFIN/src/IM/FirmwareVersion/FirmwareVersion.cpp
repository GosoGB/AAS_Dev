/**
 * @file FirmwareVersion.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-12-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Logger/Logger.h"
#include "FirmwareVersion.h"




namespace muffin {

    FirmwareVersion::FirmwareVersion(const char* semver, const uint32_t code)
        : mVersionCode(code)
    {
        SetSemanticVersion(semver);
    }

    void FirmwareVersion::SetSemanticVersion(const char* semver)
    {
        memset(mSemanticVersion, '\0', sizeof(mSemanticVersion));
        strncpy(mSemanticVersion, semver, sizeof(mSemanticVersion));
    }

    void FirmwareVersion::SetVersionCode(const uint32_t code)
    {
        mVersionCode = code;
    }

    const char* FirmwareVersion::GetSemanticVersion() const
    {
        return mSemanticVersion;
    }

    uint32_t FirmwareVersion::GetVersionCode() const
    {
        return mVersionCode;
    }


    FirmwareVersion FW_VERSION_ESP32("1.2.1", 121);
#if defined(MODLINK_T2)
    FirmwareVersion FW_VERSION_MEGA2560("0.0.0", 0);
#endif
}