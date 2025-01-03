/**
 * @file CircularBuffer.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Circular Buffer 클래스를 정의합니다.
 * 
 * @date 2024-09-08
 * @version 1.0.0
 * 
 * @note 멀티 쓰레딩 환경에서 race condition이 발생할 수 있으므로 동기화 관리가 필요합니다.
 * @todo 향후 필요한 경우에 한하여 race condition 방지를 위한 동기화 관리를 적용해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <algorithm>
#include <deque>
#include <vector>

#include "Common/Logger/Logger.h"



namespace muffin {

    class CircularBuffer
    {
    private:
        std::deque<uint8_t> mBuffer;
        size_t mCapacity;

    public:
        CircularBuffer(const size_t capacity)
            : mCapacity(capacity)
        {
        }

        ~CircularBuffer()
        {
        }

        void Write(const uint8_t value)
        {
            if (mBuffer.size() == mCapacity)
            {
                LOG_WARNING(logger, "BUFFER CAPACITY EXCEEDED. REMOVES THE OLDEST ENTRY");
                mBuffer.pop_front();
            }
            mBuffer.push_back(value);
        }

        int16_t Peek()
        {
            if (IsEmpty() == true)
            {
                LOG_WARNING(logger, "BUFFER IS EMPTY. RETURNING -1");
                return -1;
            }
            
            uint8_t value = mBuffer.front();
            return value;
        }

        int16_t Read()
        {
            if (IsEmpty() == true)
            {
                LOG_WARNING(logger, "BUFFER IS EMPTY. RETURNING -1");
                return -1;
            }

            uint8_t value = mBuffer.front();
            mBuffer.pop_front();
            return value;
        }

        size_t GetSize() const
        {
            return mBuffer.size();
        }

        bool IsEmpty() const
        {
            return mBuffer.empty();
        }

        size_t GetCapacity() const
        {
            return mCapacity;
        }

        size_t GetAvailableBytes() const
        {
            return mBuffer.size();
        }

        bool HasPattern(const std::string& pattern2Find)
        {
            if (IsEmpty() == true)
            {
                return false;
            }
            
            const std::deque<uint8_t> pattern(pattern2Find.begin(), pattern2Find.end());
            auto it = std::search(mBuffer.begin(), mBuffer.end(), pattern.begin(), pattern.end());
            return it != mBuffer.end();
        }

        size_t RemovePattern(const std::string& pattern2Remove)
        {
            if (IsEmpty() == true)
            {
                return false;
            }
            
            const std::deque<uint8_t> pattern(pattern2Remove.begin(), pattern2Remove.end());
            auto it = std::search(mBuffer.begin(), mBuffer.end(), pattern.begin(), pattern.end());
            const size_t originalSize = mBuffer.size();

            if (it != mBuffer.end())
            {
                mBuffer.erase(it, it + pattern.size());
            }
            const size_t removedSize = mBuffer.size();
        #if defined(DEBUG)
            LOG_DEBUG(logger, "Available: %u Bytes", GetAvailableBytes());
        #endif
            return originalSize - removedSize;
        }

        std::vector<uint8_t> ReadBetweenPatterns(const std::string& patternBegin, const std::string& patternEnd)
        {
            if (IsEmpty() == true)
            {
                return std::vector<uint8_t>();
            }
            
            const std::deque<uint8_t> firstPattern(patternBegin.begin(), patternBegin.end());
            const std::deque<uint8_t> lastPattern(patternEnd.begin(), patternEnd.end());

            auto firstIt = std::search(mBuffer.begin(), mBuffer.end(), firstPattern.begin(), firstPattern.end());
            if (firstIt == mBuffer.end())
            {
                return std::vector<uint8_t>();
            }
            auto endOfFirstIt = firstIt + patternBegin.size();

            auto lastIt = std::search(endOfFirstIt, mBuffer.end(), lastPattern.begin(), lastPattern.end());
            if (lastIt == mBuffer.end())
            {
                return std::vector<uint8_t>();
            }
            lastIt += patternEnd.size();

            std::vector<uint8_t> data(firstIt, lastIt);
            mBuffer.erase(firstIt, lastIt);
        #if defined(DEBUG)
            LOG_DEBUG(logger, "Available: %u Bytes", GetAvailableBytes());
        #endif
            return data;
        }
    };
}