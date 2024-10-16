/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Core.h"
#include "Include/Helper.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    Core& Core::GetInstance()
    {
        if (mInstance == nullptr)
        {
            logger = new(std::nothrow) muffin::Logger();
            if (logger == nullptr)
            {
                esp_restart();
            }
            
            mInstance = new(std::nothrow) Core();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR MUFFIN CORE");
                esp_restart();
            }
        }
        
        return *mInstance;
    }

    Core::Core()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Core::~Core()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Status Core::Init()
    {
        /**
         * @todo Reset 사유에 따라 자동으로 초기화 하는 기능의 개발이 필요합니다.
         * @details JARVIS 설정으로 인해 런타임에 크래시 같은 문제가 있을 수 있습니다.
         *          이러한 경우에는 계속해서 반복적으로 MODLINK가 리셋되는 현상이 발생할
         *          수 있습니다. 따라서 reset 사유를 확인하여 JARVIS 설정을 초기화 하는
         *          기능이 필요합니다. 단, 다른 부서와의 협의가 선행되어야 합니다.
         */
        mResetReason = esp_reset_reason();
        printResetReason(mResetReason);

        if (readMacAddressEthernet(&mMacAddressEthernet) != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO READ ETHERNET MAC ADDRESS");
            esp_restart();
        }

        ESP32FS* esp32FS = ESP32FS::GetInstance();
        if (esp32FS == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");
            esp_restart();
        }
        
        /**
         * @todo LittleFS 파티션의 포맷 여부를 로우 레벨 API로 확인해야 합니다.
         * @details 현재는 파티션 마운트에 실패할 경우 파티션을 자동으로 포맷하게
         *          코드를 작성하였습니다. 다만, 일시적인 하드웨어 실패가 발생한
         *          경우에도 파티션이 포맷되는 문제가 있습니다.
         */
        if (esp32FS->Begin(true) != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO MOUNT ESP32 FILE SYSTEM TO THE OS");
            esp_restart();
        }




        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Core* Core::mInstance = nullptr;
}