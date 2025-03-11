/**
 * @file MemoryPool.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 *
 * @brief
 *
 * @date 2025-02-04
 * @version 1.3.1
 *
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Logger/Logger.h"
#include "MemoryPool.h"



namespace muffin
{
    MemoryPool::MemoryPool(const size_t blockSize, const size_t blockCount)
        : mBlockSize(blockSize), mBlockCount(blockCount)
    {
        mPool = new char[mBlockSize * mBlockCount];
        mFreeBlocks.reserve(mBlockCount);

        for (size_t idx = 0; idx < blockCount; ++idx)
        {
            mFreeBlocks.push_back(mPool + idx * blockSize);
        }
    }

    MemoryPool::~MemoryPool()
    {
        delete[] mPool;
    }

    void* MemoryPool::Allocate(const size_t size)
    {
        const size_t blocksNeeded = (size + mBlockSize - 1) / mBlockSize;
        if (blocksNeeded > mFreeBlocks.size())
        {
            return nullptr;
        }
        
        void* block = mFreeBlocks.back();
        mFreeBlocks.pop_back();

        for (size_t i = 1; i < blocksNeeded; ++i)
        {
            mFreeBlocks.pop_back();
        }
        return block;
    }

    void MemoryPool::Deallocate(void* block, const size_t size)
    {
        size_t blocksToFree = (size + mBlockSize - 1) / mBlockSize;
        for (size_t i = 0; i < blocksToFree; ++i)
        {
            mFreeBlocks.push_back((char*)block + i * mBlockSize);
        }
    }
    
    void MemoryPool::Reset()
    {
        mFreeBlocks.clear();  // 기존의 할당된 블록 목록을 비웁니다.

        for (size_t idx = 0; idx < mBlockCount; ++idx)
        {
            mFreeBlocks.push_back(mPool + idx * mBlockSize);
        }
    }


    MemoryPool memoryPool(320, 80);
}