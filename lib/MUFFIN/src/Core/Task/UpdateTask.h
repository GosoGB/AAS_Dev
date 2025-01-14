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




namespace muffin {

    void SendStatusMSG();
    void StartManualFirmwareUpdate(const std::string& payload);
    Status HasNewFirmware(new_fw_t* outInfo);
    Status DownloadFirmware(const mcu_type_e mcu);
    bool PostDownloadResult(const mcu_type_e mcu, const std::string result);
    bool PostFinishResult(const mcu_type_e mcu, const std::string result);
    void StopAllTask();
}