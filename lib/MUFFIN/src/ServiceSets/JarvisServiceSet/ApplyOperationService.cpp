/**
 * @file ApplyOperationService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 개체를 적용하는 함수를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/TypeDefinitions.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "ServiceSets/JarvisServiceSet/ApplyOperationService.h"
#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "DataFormat/CSV/CSV.h"
#include "Protocol/SPEAR/SPEAR.h"



namespace muffin {

    void executeFactoryReset()
    {
        LOG_INFO(logger,"Received the factory reset command");
        
        Status ret = esp32FS.Format();
        if (ret == Status::Code::GOOD)
        {
            init_cfg_t initConfig;

            initConfig.PanicResetCount   = 0;
            initConfig.HasPendingJARVIS  = 0;
            initConfig.HasPendingUpdate  = 0;
            initConfig.ReconfigCode      = static_cast<uint8_t>(reconfiguration_code_e::JARVIS_USER_FACTORY_RESET);;
            const uint8_t size = 20;
            char buffer[size] = {'\0'};

            CSV csv;
            Status ret = csv.Encode(initConfig, size, buffer);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FACTORY RESET HAS FAILED, ENCODING ERROR");
                goto TEARDOWN;
            }
            
            File file = esp32FS.Open(INIT_FILE_PATH, "w", true);
            if (file == false)
            {
                LOG_ERROR(logger, "FACTORY RESET HAS FAILED, FILE OPEN ERROR");
                goto TEARDOWN;
            }

            file.write(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
            file.flush();
            file.close();
            
            char readback[size] = {'\0'};
            file = esp32FS.Open(INIT_FILE_PATH, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FACTORY RESET HAS FAILED, FILE OPEN ERROR");
                goto TEARDOWN;
            }

            file.readBytes(readback, size);
            file.close();

            if (strcmp(buffer, readback) != 0)
            {
                LOG_ERROR(logger, "FACTORY RESET HAS FAILED, FILE READ ERROR");
                goto TEARDOWN;
            }

            LOG_INFO(logger, "Factory reset has finished. Will be reset");
            goto TEARDOWN;
        }
        LOG_ERROR(logger, "FACTORY RESET HAS FAILED");
        
    TEARDOWN:
        vTaskDelay((5 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        spear.Reset();
    #endif 
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