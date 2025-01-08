/**
 * @file MemoryPool.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2024-12-29
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <new>
#include <vector>



namespace muffin {

    class MemoryPool
    {
    public:
        MemoryPool(const size_t blockSize, const size_t blockCount);
        virtual ~MemoryPool();
    public:
        void* Allocate(const size_t size);
        void  Deallocate(void* block, const size_t size);
    private:
        const size_t mBlockSize;
        const size_t mBlockCount;
        char* mPool;
        std::vector<void*> mFreeBlocks;
    };
    

    extern MemoryPool memoryPool;
}