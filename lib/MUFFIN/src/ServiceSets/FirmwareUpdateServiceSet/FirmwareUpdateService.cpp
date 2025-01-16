/**
 * @file FirmwareUpdateService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어를 업데이트하는 서비스를 선언합니다.
 * 
 * @date 2025-01-16
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "ParseFirmwareUpdateInfoService.h"



namespace muffin {

    Status FirmwareUpdateService(const char* payload)
    {
        ota::fw_info_t* esp32 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));
        ota::fw_info_t* mega2560 = (ota::fw_info_t*)malloc(sizeof(ota::fw_info_t));
        if (esp32 == nullptr || mega2560 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR FW INFO");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
    
        Status ret = ParseFirmwareUpdateInfoService(payload, esp32, mega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE UPDATE INFO");
            return ret;
        }
        LOG_INFO(logger, "Parsed successfully");
        
        // 서비스 네트워크 정보 읽어들이기
        // 서비스 네트워크에 따라 HTTP 클라이언트 생성

    #if defined(MODLINK_T2)
        if (mega2560->Head.HasNewFirmware == true)
        {
            // mega2560 업데이트 시작
                // 펌웨어 다운로드 태스크 시작
                    // 태스크 간 chunk 전달
                // 펌웨어 업데이트 태스크 시작
        }
    #endif
    
        if (esp32->Head.HasNewFirmware == true)
        {
            // esp32 업데이트 시작
                // 펌웨어 다운로드 태스크 시작
                    // 태스크 간 chunk 전달
                // 펌웨어 업데이트 태스크 시작
        }
    }
}