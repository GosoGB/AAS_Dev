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
        if (catM1 == nullptr)
        {
            catM1 = new(std::nothrow) CatM1();
            if (catM1 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
        }

        if (catM1->IsConnected() == true)
        {
            return Status(Status::Code::GOOD);
        }
        
        Status ret = catM1->Config(jvs::config::catM1);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE CatM1 CIN");
            return ret;
        }
        LOG_INFO(logger,"Configured CatM1 CIN");

        ret = catM1->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1");
            return ret;
        }

        ret = catM1->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT CatM1 MODULE TO ISP");
            return ret;
        }
        LOG_INFO(logger,"CatM1 has connected");
        
        do
        {
            if (catM1->IsConnected() == false)
            {
                ret = Status::Code::BAD_NO_COMMUNICATION;
                goto RETRY;
            }
            
            ret = catM1->SyncNTP();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

        RETRY:
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);

        } while (ret != Status::Code::GOOD);
        LOG_INFO(logger, "Synchronized with NTP server");
        
        LOG_INFO(logger,"Initialized CatM1 interface");
        return ret;
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
        
        if (ethernet->IsConnected() == true)
        {
            return Status(Status::Code::GOOD);
        }

        Status ret = ethernet->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE ETHERNET");
            return ret;
        }
        
        ret = ethernet->Config(jvs::config::ethernet);
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