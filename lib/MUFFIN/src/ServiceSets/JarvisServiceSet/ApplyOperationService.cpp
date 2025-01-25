/**
 * @file ApplyOperationService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 개체를 적용하는 함수를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "ServiceSets/JarvisServiceSet/ApplyOperationService.h"
#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    void executeFactoryReset()
    {
        LOG_INFO(logger,"Received the factory reset command");
        
        Status ret = esp32FS.Format();
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Factory reset has finished. Will be reset");
            goto TEARDOWN;
        }
        LOG_ERROR(logger, "FACTORY RESET HAS FAILED");
        
    TEARDOWN:
        vTaskDelay((5 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
        esp_restart();
    }

    Status ApplyOperationService()
    {
        if (jvs::config::operation.GetFactoryReset().second == true)
        {
            executeFactoryReset();
        }

        switch (jvs::config::operation.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            return InitCatM1Service();
    
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        case jvs::snic_e::Ethernet:
            return InitEthernetService();
    #endif
    
        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operation.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }
}