/**
 * @file CRC32.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief CRC32 체크섬을 계산하는 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Logger/Logger.h"
#include "CRC32.h"



namespace muffin {

    void CRC32::Init()
    {
        for (uint32_t i = 0; i < 256; ++i)
        {
            uint32_t crc = i;
            for (uint32_t j = 8; j > 0; --j)
            {
                if (crc & 1)
                {
                    crc = (crc >> 1) ^ 0xEDB88320;
                }
                else
                {
                    crc >>= 1;
                }
            }
            mCRC32Table[i] = crc;
        }
    }

    void CRC32::Teardown()
    {
        mTotalChecksum ^= 0xFFFFFFFF;
        LOG_DEBUG(logger, "Total CRC32: %08x", mTotalChecksum);
    }

    uint32_t CRC32::Calculate(const std::string& data)
    {
        mChunkChecksum = 0xFFFFFFFF;
        for (size_t i = 0; i < data.length(); ++i)
        {
            uint8_t byte = static_cast<uint8_t>(data[i]);
            uint32_t tableIndexCurrent = (mChunkChecksum ^ byte) & 0xFF;
            uint32_t tableIndexTotal   = (mTotalChecksum ^ byte) & 0xFF;
            mChunkChecksum = (mChunkChecksum >> 8) ^ mCRC32Table[tableIndexCurrent];
            mTotalChecksum = (mTotalChecksum >> 8) ^ mCRC32Table[tableIndexTotal];
        }
        mChunkChecksum ^= 0xFFFFFFFF;
        LOG_DEBUG(logger, "Calculated CRC32: %08x", mChunkChecksum);
        return mChunkChecksum;
    }

    uint32_t CRC32::Calculate(const size_t size, uint8_t data[])
    {
        mChunkChecksum = 0xFFFFFFFF;
        for (size_t i = 0; i < size; ++i)
        {
            uint8_t byte = data[i];
            uint32_t tableIndexCurrent = (mChunkChecksum ^ byte) & 0xFF;
            uint32_t tableIndexTotal   = (mTotalChecksum ^ byte) & 0xFF;
            mChunkChecksum = (mChunkChecksum >> 8) ^ mCRC32Table[tableIndexCurrent];
            mTotalChecksum = (mTotalChecksum >> 8) ^ mCRC32Table[tableIndexTotal];
        }
        mChunkChecksum ^= 0xFFFFFFFF;
        LOG_DEBUG(logger, "Calculated CRC32: %08x", mChunkChecksum);
        return mChunkChecksum;
    }

    uint32_t CRC32::RetrieveTotalChecksum() const
    {
        return mTotalChecksum;
    }

    void CRC32::Reset()
    {
        mTotalChecksum = 0xFFFFFFFF;
        mChunkChecksum = 0xFFFFFFFF;
    }
}