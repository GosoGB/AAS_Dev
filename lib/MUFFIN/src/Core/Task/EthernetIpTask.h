/**
 * @file EthernetIpTask.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JARIVS 설정 정보를 토대로 Ethernet/IP 프로토콜로 데이터를 수집하는 태스크를 정의합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#include "Protocol/EthernetIP/EthernetIP.h"


namespace muffin {

    void StartEthernetIpTask();
    void StopEthernetIpTask();
    bool HasEthernetIpTask();

    extern std::vector<ethernetIP::EthernetIP> EthernetIpVector;

}



#endif