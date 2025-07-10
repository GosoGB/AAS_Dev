/**
 * @file AddressArrayTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 AddressArrayTable 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#pragma once

#include <vector>
#include <string>

#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"


namespace muffin { namespace ethernetIP {

    
    typedef struct TagArrayEntry
    {
        std::string tagName;
        size_t startIndex;
        size_t count;
    } tag_array_entry_t;

    class AddressArrayTable
    {
    public:
        AddressArrayTable();
        ~AddressArrayTable();

    public:
        Status Update(const std::string& tagName, size_t startIndex, size_t count);
        Status Remove(const std::string& tagName, size_t startIndex, size_t count);
        std::vector<tag_array_entry_t> RetrieveTable() const;
        size_t GetArrayBatchCount() const;
        void DebugPrint() const;


    private:
        std::vector<tag_array_entry_t> mArrayTable;

    };
}}


#endif