/**
 * @file FirmwareVersion.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 펌웨어 버전 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.hpp"
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
        ASSERT((strlen(semver) < sizeof(mSemanticVersion)), "BUFFER OVERFLOW: SEMANTIC VERSION");
        
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


	FirmwareVersion FW_VERSION_ESP32(ESP32_FW_VERSION, ESP32_FW_VERSION_CODE);
#if defined(MT10)
	FirmwareVersion FW_VERSION_MEGA2560(MEGA_FW_VERSION, MEGA_FW_VERSION_CODE);
#endif
}