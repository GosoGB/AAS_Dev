// /**
//  * @file AddressArrayTable.cpp
//  * @author Kim, Joo-sung (joosung5732@edgecross.ai)
//  * 
//  * @brief Ethernet/IP 프로토콜 전용의 AddressArrayTable 정의합니다. 
//  * 
//  * @date 2025-07-01
//  * @version 1.5.0
//  * 
//  * @copyright Copyright Edgecross Inc. (c) 2025
//  */


// #if defined(MT11)

// #include <HardwareSerial.h>

// #include "Common/Assert.h"
// #include "Common/Logger/Logger.h"
// #include "Common/Time/TimeUtils.h"
// #include "Common/Convert/ConvertClass.h"
// #include "AddressArrayTable.h"



// namespace muffin { namespace ethernetIP {

//     AddressArrayTable::AddressArrayTable()
//     {

//     }

//     AddressArrayTable::~AddressArrayTable()
//     {

//     }

//     Status AddressArrayTable::Update(const std::string& tagName, size_t startIndex, size_t count)
//     {
//         if (count == 0)
//         {
//             LOG_ERROR(logger,"COUNT MUST NOT BE A ZERO");
//             return Status(Status::Code::BAD); 
//         }

//         if (!tryMergeLast(tagName, startIndex, count)) 
//         {
//             tag_array_entry_t entry = { tagName, startIndex, count };
//             mArrayTable.emplace_back(entry);
//         }

//         return Status(Status::Code::GOOD);
//     }

//     Status AddressArrayTable::Remove(const std::string& tagName, size_t startIndex, size_t count)
//     {
        
//         return Status(Status::Code::BAD);
//     }

//     bool AddressArrayTable::tryMergeLast(const std::string& tagName, size_t startIndex, size_t count)
//     {
//         if (mArrayTable.empty()) return false;

//         for (auto& entry : mArrayTable) 
//         {
//             if (entry.tagName != tagName) continue;

//             uint16_t entryEnd = entry.startIndex + entry.count;

//             if (startIndex + count == entry.startIndex)     
//             {
//                 entry.startIndex = startIndex;
//                 entry.count += count;
//                 return true;
//             }

//             if (entryEnd == startIndex) 
//             {
//                 entry.count += count;
//                 return true;
//             }
//         }

//         return false;
//     }

    
// }}













// #endif