/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Core.h"
#include "Include/Helper.h"
#include "Initializer/Initializer.h"


// #include "IM/MacAddress/MacAddress.h"





namespace muffin {

    Core& Core::GetInstance() noexcept
    {
        if (mInstance == nullptr)
        {
            logger = new(std::nothrow) muffin::Logger();
            if (logger == nullptr)
            {
                ASSERT(false, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR LOGGER");
                esp_restart();
            }
            
            mInstance = new(std::nothrow) Core();
            if (mInstance == nullptr)
            {
                ASSERT(false, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR MUFFIN CORE");
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
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
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

        Initializer initializer;
        initializer.StartOrCrash();

        Status ret = initializer.Configure();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE MUFFIN: %s", ret.c_str());
            성공할 때까지 다시 한 번 시도합니다.
            아니면 그냥 디바이스를 리셋하는 게 나을지도 모릅니다...
        }
        


        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }


    Core* Core::mInstance = nullptr;
}