/**
 * @file AddressTable.cpp
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

#include <HardwareSerial.h>
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "AddressTable.h"


namespace muffin { namespace ethernetIP {



    AddressTable::AddressTable(size_t maxSize)
    : maxBatchSize(maxSize)
    {
        mBatches.emplace_back();
    }

    AddressTable::~AddressTable()
    {

    }

    void AddressTable::Clear()
    {
        mBatches.clear();
    }

    Status AddressTable::Update(const std::string& tag)
    {
        size_t tagSize = estimateTagSize(tag);

        // 이미 존재하는지 확인 (중복 방지)
        if (contains(tag) == Status::Code::GOOD)
        {     
            return Status(Status::Code::GOOD_NO_DATA);
        }

        // 현재 배치 초과 시 새 배치 생성
        if (mBatches.back().totalSize + tagSize > maxBatchSize) 
        {
            mBatches.emplace_back();
        }

        tag_batch_struct_t& current = mBatches.back();
        current.tags.push_back(tag);
        current.totalSize += tagSize;

        return Status(Status::Code::GOOD);
    }

    Status AddressTable::Remove(const std::string& tag)
    {
        for (auto it = mBatches.begin(); it != mBatches.end(); ++it) 
        {
            auto& tags = it->tags;
            for (auto tagIt = tags.begin(); tagIt != tags.end(); ++tagIt) 
            {
                if (*tagIt == tag) 
                {
                    size_t tagSize = estimateTagSize(tag);
                    tags.erase(tagIt);
                    it->totalSize -= tagSize;

                    // 빈 배치 정리
                    if (tags.empty()) {
                        mBatches.erase(it);
                    }
                    
                    return Status(Status::Code::GOOD);
                }
            }
        }

        //상태 값 확인해서 리턴하기
        return Status(Status::Code::BAD);
    }

    std::vector<std::string> AddressTable::RetrieveTagsByBatch(size_t batchIndex) const 
    {
        try 
        {
            return mBatches.at(batchIndex).tags;
        } 
        catch (const std::out_of_range& e) 
        {
            LOG_WARNING(logger, "BATCH INDEX DOES NOT EXIST");
            return {};
        }
    }

    size_t AddressTable::GetBatchCount() const 
    {
        return mBatches.size();
    }

    void AddressTable::DebugPrint() const 
    {
        Serial.println(F("\n======================="));
        Serial.println(F("🧩 Batch Table Summary"));
        Serial.println(F("=======================\n"));

        for (size_t i = 0; i < mBatches.size(); ++i) 
        {
            const tag_batch_struct_t& batch = mBatches[i];
            Serial.printf("📦 Batch %2d | Size: %4d bytes | Tags: %2d\n", (int)i + 1, (int)batch.totalSize, (int)batch.tags.size());
            Serial.println(F("────────────────────────────────────"));

            for (size_t j = 0; j < batch.tags.size(); ++j) {
                Serial.printf("  %2d. %s\n", (int)j + 1, batch.tags[j].c_str());
            }

            Serial.println();
        }

        Serial.printf("✅ Total Batches: %d\n", (int)mBatches.size());
        Serial.println(F("=======================\n"));
    }

    size_t AddressTable::estimateTagSize(const std::string& tag) const 
    {
        size_t nameLen = tag.length();
        size_t cipPath = 2 + nameLen + (nameLen % 2);  // 0x91 + len + name + padding
        size_t overhead = 4;  // 추정 오버헤드
        return cipPath + overhead;
    }

    Status AddressTable::contains(const std::string& tag) const 
    {
        for (const auto& batch : mBatches) 
        {
            for (const auto& existing : batch.tags) 
            {
                if (existing == tag)
                {
                    return Status(Status::Code::GOOD);
                }
            }
        }
        return Status(Status::Code::BAD);
    }

}}

#endif