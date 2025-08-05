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

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Protocol/EthernetIP/ciplibs/cip_util.h"
#include "PolledArrayDataTable.h"



namespace muffin { namespace ethernetIP {

    PolledArrayDataTable::PolledArrayDataTable() 
    {

    }

    PolledArrayDataTable::~PolledArrayDataTable() 
    {

    }

    Status PolledArrayDataTable::UpdateArrayRange(const std::string& tagName, size_t startIndex, const psramVector<cip_data_t>& values)
    {
        if (values.empty()) return Status(Status::Code::BAD);

        auto& indexMap = mTagArrayData[tagName];
        for (size_t i = 0; i < values.size(); ++i)
        {
            indexMap[startIndex + i] = values[i];
        }
        return Status(Status::Code::GOOD);
    }

    Status PolledArrayDataTable::GetArrayRange(const std::string& tagName, size_t startIndex, size_t count, psramVector<cip_data_t>& outValues) const
    {
        outValues.clear();

        auto tagIt = mTagArrayData.find(tagName);
        if (tagIt == mTagArrayData.end())
        {
            return Status(Status::Code::BAD);

        } 
        const auto& indexMap = tagIt->second;

        for (size_t i = 0; i < count; ++i)
        {
            size_t index = startIndex + i;
            auto it = indexMap.find(index);
            if (it != indexMap.end())
            {
                outValues.emplace_back(it->second);
            }
            else
            {
                cip_data_t dummy{};
                outValues.emplace_back(dummy); // 또는 실패로 처리할 수도 있음
            }
        }
        return Status(Status::Code::GOOD);
    }

    void PolledArrayDataTable::Clear()
    {
        mTagArrayData.clear();
    }


}}



#endif