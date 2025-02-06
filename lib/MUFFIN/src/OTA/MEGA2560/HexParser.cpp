/**
 * @file HexParser.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Intel Hex 형식의 데이터를 파싱 하는 클래스를 정의합니다.
 * 
 * @date 2024-11-28
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#if defined(MODLINK_T2)




#include <sstream>

#include <Common/Assert.h>
#include <Common/Convert/ConvertClass.h>
#include <Common/Logger/Logger.h>
#include "HexParser.h"



namespace muffin { namespace ota {

    Status HexParser::Parse(const std::string& chunk)
    {
        mReceivedData.append(chunk);
        mReceivedData.shrink_to_fit();
        // LOG_DEBUG(logger, "mReceivedData: %s", mReceivedData.c_str());

        try
        {
            while (true)
            {
                const size_t pos = mReceivedData.find_first_of("\r\n");
                if (pos == std::string::npos)
                {
                    if ((mReceivedData.length() < 12) && (mReceivedData == ":00000001FF"))
                    {
                        // LOG_DEBUG(logger, "Received the EOF record: %s", mReceivedData.c_str());
                        mHexRecords.emplace_back(mReceivedData);
                        mReceivedData.clear();
                        break;
                    }
                    else
                    {
                        // LOG_DEBUG(logger, "Insufficient Line: %s", mReceivedData.c_str());
                        break;
                    }
                }
                
                const std::string line = mReceivedData.substr(0, pos);
                // LOG_DEBUG(logger, "Line: %s", line.c_str());
                mHexRecords.emplace_back(line);
                mReceivedData.erase(0, pos + CR_LF_LENGTH);
            }
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE HEX RECORDS: %s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE HEX RECORDS: %s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }


        page_t page;
        memset(page.Data, 0, sizeof(page.Data));
        page.Size = 0;


        for (auto it = mHexRecords.begin(); it != mHexRecords.end(); ++it)
        {
            if ((it->length() < 12) && (it->find(":00000001FF") != std::string::npos))
            {
                // LOG_DEBUG(logger, "Received the EOF record");
                if (page.Size % PAGE_SIZE != 0)
                {
                    for (uint16_t i = page.Size; i < PAGE_SIZE; ++i)
                    {
                        page.Data[i] = 0xFF;
                        ++page.Size;
                    }
                    // LOG_DEBUG(logger, "Paddings embedded to the page");
                }
                mPages.emplace_back(page);
                // LOG_DEBUG(logger, "Record Size: %u", mHexRecords.size());
                // LOG_DEBUG(logger, "EOF Record: %s", mHexRecords.front().c_str());
                
                page.Size = 0;
                mHexRecords.erase(mHexRecords.begin(), it);
                break;
            }

            // LOG_DEBUG(logger, "HEX Record: %s", it->c_str());
            const uint8_t endIndex = it->length() - CHECKSUM_CHAR_LENGTH;
            const uint8_t length = endIndex - START_INDEX;
            std::string parsedData = it->substr(START_INDEX, length);
            // LOG_DEBUG(logger, "Parsed Data: %s", parsedData.c_str());

            for (uint16_t i = 0; i < length; i += 2)
            {
                page.Data[page.Size++] = strtol(parsedData.substr(i, 2).c_str(), NULL, HEX);
                if (page.Size % PAGE_SIZE == 0)
                {
                    mPages.emplace_back(page);
                    // LOG_DEBUG(logger, "Emplaced a new page: %u pages", mPages.size());
                    // LOG_DEBUG(logger, "mHexRecords: %u", mHexRecords.size());
                    // LOG_DEBUG(logger, "remained: %u", ESP.getFreeHeap());

                    if (std::next(it) == mHexRecords.end())
                    {
                        mHexRecords.clear();
                    }
                    else
                    {
                        it = mHexRecords.erase(mHexRecords.begin(), std::next(it));
                        it = std::prev(it);
                    }
                    // LOG_DEBUG(logger, "mHexRecords: %u", mHexRecords.size());
                    page.Size = 0;
                }
            }

            if (mHexRecords.size() == 0)
            {
                return Status(Status::Code::GOOD_MORE_DATA);
            }
        }
        // LOG_DEBUG(logger, "mHexRecords: %u", mHexRecords.size());
        // LOG_DEBUG(logger, "mPages: %u", mPages.size());

        if (page.Size != 0)
        {
            // LOG_DEBUG(logger, "Insufficient records to make a new page");
            return Status(Status::Code::GOOD_MORE_DATA);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }

    size_t HexParser::GetPageCount() const
    {
        return mPages.size();
    }

    page_t HexParser::GetPage()
    {
        page_t page = mPages.front();
        // LOG_DEBUG(logger, "Size: %u", page.Size)
        return page;
    }

    void HexParser::RemovePage()
    {
        mPages.erase(mPages.begin());
    }
}}



#endif