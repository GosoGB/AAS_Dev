/**
 * @file EthernetIP.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MFM으로 설정한 Ethernet/IP 프로토콜에 따라 데이터를 수집, 처리, 관리하는 클래스를 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */




#if defined(MT11)

#pragma once

#include "NodeTable.h"
#include "AddressTable.h"
#include "Protocol/EthernetIP/ciplibs/cip_client.h"
#include "Protocol/EthernetIP/ciplibs/cip_msr.h"
#include "Protocol/EthernetIP/ciplibs/cip_single.h"
#include "Protocol/EthernetIP/ciplibs/cip_types.h"
#include "Protocol/EthernetIP/ciplibs/cip_path.h"
#include "Protocol/EthernetIP/ciplibs/cip_util.h"
#include "Protocol/EthernetIP/ciplibs/eip_session.h"
#include "Protocol/EthernetIP/ciplibs/eip_connection.h"
#include "Protocol/EthernetIP/ciplibs/eip_types.h"
#include "JARVIS/Config/Protocol/EthernetIP.h"


namespace muffin { namespace ethernetIP {


    class EthernetIP
    {
    public:
        EthernetIP();
        virtual ~EthernetIP();

    public:
        Status Config(jvs::config::EthernetIP *config);
        void Clear();
        IPAddress GetServerIP();
        uint16_t GetServerPort();
        bool Connect();
        Status Poll();
        void SetTimeoutError(); 

        
    private:
        EIPSession mSession;
    
    private:
        NodeTable mNodeTable;
        AddressTable mAddressTable;
    

    };
    
}}
#endif