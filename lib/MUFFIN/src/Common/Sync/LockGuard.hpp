/**
 * @file LockGuard.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief A class that provides a lock guard mechanism for managing mutex locks.
 * 
 * @date 2025-06-17
 * @version 1.4.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */



#pragma once

#include "Mutex.hpp"



class LockGuard
{
public:
    explicit LockGuard(Mutex& mutex, const TickType_t timeoutTicks = portMAX_DELAY)
        : mMutex(mutex)
    {
        mIsLocked = mMutex.Lock(timeoutTicks);
    }

    ~LockGuard()
    {
        mMutex.Unlock();
    }

    bool IsLocked() const
    {
        return mIsLocked;
    }

private:
    Mutex& mMutex;
    bool mIsLocked = false;
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};
