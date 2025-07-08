/**
 * @file AddressTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 AddressTable을 정의합니다. 
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

    typedef struct TagBatch 
    {
        std::vector<std::string> tags;
        size_t totalSize = 0;   
    } tag_batch_struct_t ;


    class AddressTable
    {
    public:
        AddressTable(size_t maxSize = 400);
        virtual ~AddressTable();

    public:
        Status Update(const std::string& tag);
        Status Remove(const std::string& tag);
        void Clear();
        std::vector<std::string> RetrieveTagsByBatch(size_t batchIndex) const;
        size_t GetBatchCount() const;
        void DebugPrint() const;

        
    private:
        size_t estimateTagSize(const std::string& tag) const;
        Status contains(const std::string& tag) const;

    private:
        std::vector<tag_batch_struct_t> mBatches;
        size_t maxBatchSize;
    
    };

}}


#endif