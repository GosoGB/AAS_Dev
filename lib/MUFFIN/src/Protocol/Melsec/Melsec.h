/**
 * @file Melsec.h
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec 프로토콜 클래스를 선언합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#pragma once


#include <vector>

#include "Common/Status.h"
#include "Protocol/Modbus/Include/AddressTable.h"
#include "Protocol/Modbus/Include/NodeTable.h"
#include "Protocol/Modbus/Include/PolledDataTable.h"
#include "Protocol/Modbus/Include/TypeDefinitions.h"
#include "JARVIS/Config/Protocol/Melsec.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Protocol/Melsec/MelsecClient.h"
#if defined(MT11)
    #include "Network/Ethernet/W5500/EthernetClient.h"
#else
#endif

namespace muffin {

    class Melsec
    {
    public:
        Melsec();
        virtual ~Melsec();
    private:
        using AddressRange = im::NumericAddressRange;

    public:
#if defined(MT11)
        Status SetClient(MelsecClient* client);
#endif
        Status Config(jvs::config::Melsec* config);
        void Clear();
        IPAddress GetServerIP();
        uint16_t GetServerPort();
        bool Connect();
        Status Poll();
        void SetTimeoutError();
    private:
        Status addNodeReferences(const uint8_t slaveID, const std::vector<std::__cxx11::string>& vectorNodeID);
        im::NumericAddressRange createAddressRange(const uint16_t address, const uint16_t quantity) const;
        Status implementPolling();
        Status updateVariableNodes();
    private:
    // @lsj 이건 어떤 naming convention을 따른 거냐의 문제인데
    //      저는 개인적으로 함수는 동사로 시작해야 한다고 생각해요!?
        Status bitsRead(const jvs::node_area_e area, const std::set<AddressRange>& addressRangeSet);
        Status wordsRead(const jvs::node_area_e area, const std::set<AddressRange>& addressRangeSet);
    public:
        modbus::datum_t GetAddressValue(const uint8_t slaveID, const uint16_t address, const jvs::node_area_e area);
    
    public:
        MelsecClient* mMelsecClient = nullptr;

    private:
        modbus::NodeTable mNodeTable;
        modbus::AddressTable mAddressTable;
        modbus::PolledDataTable mPolledDataTable;
    
    private:
        IPAddress mServerIP;
        uint16_t mServerPort;
        jvs::ps_e mPlcSeries;
        jvs::df_e mDataformat;
        uint16_t mScanRate;

        static constexpr uint8_t DEFAULT_SLAVE_NUMBER = 1;
    };
    
   
    

}