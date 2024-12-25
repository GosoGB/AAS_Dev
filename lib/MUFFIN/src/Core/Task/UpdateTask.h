/**
 * @file UpdateTask.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 주기를 확인하며 주기 데이터를 생성해 CDO로 전달하는 기능의 TASK를 구현합니다.
 * 
 * @date 2024-11-29
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <vector>

#include "Common/Status.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"



namespace muffin {

    typedef enum McuTypeEnum mcu_type_e;

    typedef struct NewFirmwareInfoType
    {
        bool MCU_ESP32;
        bool MCU_MEGA2560;
    } new_fw_t;

    typedef struct UrlInfoType
    {
        http_scheme_e Scheme;
        std::string Host;
        uint16_t Port;
    } url_t;
 
    typedef struct McuInfo
    {
        uint32_t VersionCode;
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

    void StartUpdateTask();
    void StartManualFirmwareUpdate(const std::string& payload);
    void UpdateTask(void* pvParameter);
    Status HasNewFirmware(new_fw_t* outInfo);
    Status DownloadFirmware(const mcu_type_e mcu);
    bool PostDownloadResult(const mcu_type_e mcu, const std::string result);
    bool PostFinishResult(const mcu_type_e mcu, const std::string result);
    void StopAllTask();
}