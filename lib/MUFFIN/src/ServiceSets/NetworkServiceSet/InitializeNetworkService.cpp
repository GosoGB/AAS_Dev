/**
 * @file InitializeNetworkService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 초기화 함수를 정의합니다.
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
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"



namespace muffin {

    


    Status InitCatM1Service()
    {
        CatM1& catM1 = CatM1::GetInstance();
        std::pair<muffin::Status, size_t> mutex = catM1.TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX: %s", mutex.first.c_str());
            return;
        }
        ;
    }

    Status InitEthernetService()
    {
        if (ethernet == nullptr)
        {
            ethernet = new(std::nothrow) Ethernet();
            if (ethernet == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }
        
        Status ret = ethernet->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE ETHERNET");
            return ret;
        }
        
        ret = ethernet->Config(jvs::config::ethernetCIN);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE ETHERNET CIN");
            return ret;
        }
        LOG_INFO(logger,"Configured ethernet CIN");

        ret = ethernet->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT ETHERNET");
            return ret;
        }
        LOG_INFO(logger,"Ethernet has connected");
        
        do
        {
            ret = ethernet->SyncNTP();
        } while (ret != Status::Code::GOOD);
        LOG_INFO(logger, "Synchronized with NTP server");

        LOG_INFO(logger,"Initialized ethernet interface");
        return ret;
    }

    Status InitWiFiService()
    {
        ASSERT((false), "Wi-Fi INTERFACE IS NOT SUPPORTED YET");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}