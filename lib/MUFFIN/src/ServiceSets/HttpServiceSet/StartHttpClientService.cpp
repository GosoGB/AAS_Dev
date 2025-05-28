/**
 * @file StartHttpClientService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief HTTP 클라이언트 초기화 및 연결하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Protocol/HTTP/LwipHTTP/LwipHTTP.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "ServiceSets/HttpServiceSet/StartHttpClientService.h"



namespace muffin {

    Status strategyInitCatM1(const size_t mutex)
    {
        http::CatHTTP* catHTTP = new(std::nothrow) http::CatHTTP();
        if (catHTTP == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CatM1 HTTTP CLIENT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        Status ret = catHTTP->Init(mutex, network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_1);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized CatM1 HTTP Client");
            httpClient = catHTTP;
        }
        else
        {
            delete catHTTP;
            catHTTP = nullptr;
            LOG_ERROR(logger, "FAILED TO INITIALIZE CatM1 HTTTP Client: %s", ret.c_str());
        }

        return ret;
    }

#if defined(MT10) || defined(MB10) || defined(MT11)
    static Status strategyInitEthernet()
    {
        http::LwipHTTP* lwipHTTP = new(std::nothrow) http::LwipHTTP();
        if (lwipHTTP == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR LwIP HTTTP CLIENT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        Status ret = lwipHTTP->Init();
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Initialized LwIP HTTP Client");
            httpClient = lwipHTTP;
        }
        else
        {
            delete lwipHTTP;
            lwipHTTP = nullptr;
            LOG_ERROR(logger, "FAILED TO INITIALIZE LwIP HTTTP Client: %s", ret.c_str());
        }
        
        return ret;
    }
#endif

    Status InitHttpService()
    {
        if (httpClient != nullptr)
        {
            return Status(Status::Code::GOOD);
        }
        
        std::pair<Status, size_t> mutex = std::make_pair(Status(Status::Code::BAD), 0);
        if (jvs::config::operation.GetServerNIC().second == jvs::snic_e::LTE_CatM1)
        {
            mutex = catM1->TakeMutex();
            if (mutex.first != Status::Code::GOOD)
            {
                return mutex.first;
            }
        }
        
        Status ret(Status::Code::UNCERTAIN);
        switch (jvs::config::operation.GetServerNIC().second)
        {
        case jvs::snic_e::LTE_CatM1:
            ret = strategyInitCatM1(mutex.second);
            catM1->ReleaseMutex();
            return ret;
    #if defined(MT10) || defined(MB10) || defined(MT11)
        case jvs::snic_e::Ethernet:
            return strategyInitEthernet();
    #endif
        
        default:
            ASSERT(false, "UNDEFINED SNIC: %u", static_cast<uint8_t>(jvs::config::operation.GetServerNIC().second));
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }
}