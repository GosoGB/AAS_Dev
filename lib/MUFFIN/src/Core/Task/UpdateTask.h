/**
 * @file UpdateTask.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-10-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <vector>

namespace muffin {

    void StartManualFirmwareUpdate(JsonDocument& doc);
    void StartUpdateTask();
    void UpdateTask(void* pvParameter);
    bool HasNewFirmwareFOTA();
    bool DownloadFirmware();
    bool PostDownloadResult(const std::string resultStr);
    bool PostFinishResult(const std::string resultStr);
    bool UpdateFirmware();
    void SendStatusMSG();
    void StopAllTask();
 
    typedef struct McuInfo
    {
        uint16_t VersionCode;
        std::string FirmwareVersion;
        std::vector<uint8_t> FileNumberVector;
        std::vector<std::string> FilePathVector;
        std::vector<uint32_t> FileSizeVector;
        uint64_t FileTotalSize;
        std::vector<std::string> FileChecksumVector;
        std::string FileTotalChecksum;
    } mcu_t;
    
    typedef struct FotaInfo
    {
        uint8_t OtaID;
        mcu_t mcu1;
        mcu_t mcu2;
    } fota_Info_t;

    constexpr uint8_t FIRMWARE_VERSION_CODE_MCU1 = 6;
    const std::string FIRMWARE_VERSION_MCU1 = "0.0.6";
    constexpr uint8_t MAX_RETRY_COUNT = 5;
    
    extern std::string FotaHost;
    extern uint16_t FotaPort;
// #if !defined(DEBUG)
//     OTA 서버 릴리즈 주소 확인해야 함!!!
// #endif
}