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
#include "AddressArrayTable.h"
#include "PolledArrayDataTable.h"
#include "Common/Allocator/psramAllocator.h"
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
        EthernetIP(EIPSession EipSession);
        virtual ~EthernetIP();

    public:
        Status Config(jvs::config::EthernetIP *config);
        void Clear();
        IPAddress GetServerIP();
        uint16_t GetServerPort();
        bool Connect();
        Status Poll();
        void SetTimeoutError(); 
        cip_data_t GetSingleAddressValue(std::string tag);
    
    private:
        Status addNodeReferences(const std::vector<std::string>& vectorNodeID);
        Status implementPolling();
        Status updateVariableNodes();

    public:
        Status cipDataConvertToPollData(cip_data_t& data, im::poll_data_t* output);
        Status StringConvertToCipData(std::string& data, cip_data_t* output);
    public:
        EIPSession mEipSession;
    
    private:
        IPAddress mServerIP;
        uint16_t mServerPort;
    
    private:
        NodeTable mNodeTable;
        AddressTable mAddressTable;
        AddressArrayTable mAddressArrayTable;
        std::map<std::string, cip_data_t> mPolledDataTable;
        PolledArrayDataTable mPolledArrayDataTable;
        uint16_t mScanRate;
    private:
        bool isPossibleToConvert(std::string& data);

    };
    
}}
#endif