/**
 * @file FirmwareVersion.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 펌웨어 버전 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
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