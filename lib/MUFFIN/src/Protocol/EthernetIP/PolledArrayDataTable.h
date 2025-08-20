/**
 * @file PolledArrayDataTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 PolledArrayDataTable 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#pragma once


#include <map>
#include <vector>
#include <string>

#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Common/PSRAM.hpp"
#include "Protocol/EthernetIP/ciplibs/cip_types.h"



namespace muffin { namespace ethernetIP {

    class PolledArrayDataTable
    {
    public:
        PolledArrayDataTable();
        ~PolledArrayDataTable();
    
    public:
        Status UpdateArrayRange(const std::string& tagName, size_t startIndex, const psram::vector<cip_data_t>& values);
        Status GetArrayRange(const std::string& tagName, size_t startIndex, size_t count, psram::vector<cip_data_t>& outValues) const;
        void Clear();

    private:
        psram::map<std::string, psram::map<size_t, cip_data_t>> mTagArrayData;
    };

}}


#endif