/**
 * @file AddressArrayTable.cpp
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

#include <HardwareSerial.h>
#include <algorithm>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "AddressArrayTable.h"



namespace muffin { namespace ethernetIP {

    AddressArrayTable::AddressArrayTable()
    {

    }

    AddressArrayTable::~AddressArrayTable()
    {

    }

    Status AddressArrayTable::Update(const std::string& tagName, size_t startIndex, size_t count)
    {
        if (count == 0)
        {
            LOG_ERROR(logger,"COUNT MUST NOT BE A ZERO");
            return Status(Status::Code::BAD); 
        }

        tag_array_entry_t entry = { tagName, startIndex, count };
        mArrayTable.emplace_back(entry);

        return Status(Status::Code::GOOD);
    }

    Status AddressArrayTable::Remove(const std::string& tagName, size_t startIndex, size_t count)
    {
        for (size_t i = 0; i < mArrayTable.size(); ++i)
        {
            const tag_array_entry_t& entry = mArrayTable[i];

            if (entry.tagName == tagName &&
                entry.startIndex == startIndex &&
                entry.count == count)
            {
                mArrayTable.erase(mArrayTable.begin() + i);
                return Status(Status::Code::GOOD);
            }
        }

        LOG_WARNING(logger, "Exact matching entry not found for removal");
        return Status(Status::Code::GOOD_NO_DATA);
    }

    std::vector<tag_array_entry_t> AddressArrayTable::RetrieveTable() const
    {
        return mArrayTable;
    }

    size_t AddressArrayTable::GetArrayBatchCount() const 
    {
        return mArrayTable.size();
    }

    void AddressArrayTable::DebugPrint() const
    {
        Serial.println();
        Serial.println(F("================== AddressArrayTable ========================"));
        Serial.println(F("|               Tag Name                   | Start | Count  |"));
        Serial.println(F("|------------------------------------------|-------|--------|"));
        
        for (const auto& entry : mArrayTable)
        {
            // 태그 최대 40자 출력, 숫자는 오른쪽 정렬
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "| %-40s | %5d | %6d |",
                    entry.tagName.c_str(),
                    static_cast<int>(entry.startIndex),
                    static_cast<int>(entry.count));
            Serial.println(buffer);
        }

        Serial.println(F("============================================================="));
        Serial.println();
    }


    
}}













#endif