/**
 * @file Container.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-11
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include <esp32-hal-log.h>

#include "../Container.hpp"



namespace muffin { namespace aas {


    Container* Container::GetInstance()
    {
        if (mInstance == nullptr)
        {
            void* allocatedMemory = psram::allocate(sizeof(Container));
            if (allocatedMemory == nullptr)
            {
                log_e("BAD OUT OF MEMORY");
                return nullptr;
            }

            mInstance = new(allocatedMemory) Container();
            mInstance->mVectorAAS.reserve(2);
            mInstance->mVectorSubmodel.reserve(5);
        }

        return mInstance;
    }


    Container* Container::mInstance = nullptr;
}}