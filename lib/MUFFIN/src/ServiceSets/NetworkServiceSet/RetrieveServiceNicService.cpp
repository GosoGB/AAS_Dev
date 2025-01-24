/**
 * @file RetrieveServiceNicService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정을 따라 서비스 네트워크를 반환하는 서비스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"



namespace muffin {

    INetwork* RetrieveServiceNicService()
    {
        const jvs::snic_e snicType = jvs::config::operation.GetServerNIC().second;

        switch (snicType)
        {
        case jvs::snic_e::LTE_CatM1:
            return static_cast<INetwork*>(catM1);

    #if defined(MODLINK_T2) || defined(MODLINK_B)
        case jvs::snic_e::Ethernet:
            return static_cast<INetwork*>(ethernet);
    #endif
        
        default:
            ASSERT(false, "UNDEFINED SERVICE NETWORK TYPE: %u", static_cast<uint8_t>(snicType));
            return nullptr;
        }
    }
}