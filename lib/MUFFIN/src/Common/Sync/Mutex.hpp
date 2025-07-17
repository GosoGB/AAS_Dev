/**
 * @file Mutex.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief A class that provides a mutex lock mechanism for thread synchronization.
 * 
 * @date 2025-06-17
 * @version 1.4.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "Common/Assert.h"



class Mutex
{
public:
    Mutex()
        : mHandle(nullptr)
    {
        mHandle = xSemaphoreCreateMutex();
        ASSERT((mHandle != nullptr), "FAILED TO CREATE MUTEX HANDLE");
    }

    ~Mutex()
    {
        if (mHandle != nullptr)
        {
            vSemaphoreDelete(mHandle);
        }
    }

    bool Lock(const TickType_t timeoutTicks = portMAX_DELAY)
    {
        return xSemaphoreTake(mHandle, timeoutTicks) == pdTRUE;
    }

    void Unlock()
    {
        xSemaphoreGive(mHandle);
    }

    SemaphoreHandle_t GetHandle() const
    {
        return mHandle;
    }

private:
    SemaphoreHandle_t mHandle;
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};