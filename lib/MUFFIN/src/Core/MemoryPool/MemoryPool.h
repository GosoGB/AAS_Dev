/**
 * @file MemoryPool.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-02-04
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <new>
#include <vector>
#include "Common/PSRAM.hpp"


namespace muffin {

    class MemoryPool
    {
    public:
        MemoryPool(const size_t blockSize, const size_t blockCount);
        virtual ~MemoryPool();
    public:
        void* Allocate(const size_t size);
        void  Deallocate(void* block, const size_t size);
        void  Reset();
    private:
        const size_t mBlockSize;
        const size_t mBlockCount;
        char* mPool;
    #if defined(MT11)
        psram::vector<void*> mFreeBlocks;
    #else
        std::vector<void*> mFreeBlocks;
    #endif
        
    };
    

    extern MemoryPool memoryPool;
}