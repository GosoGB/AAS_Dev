/**
 * @file microSD.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-11
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "./microSD.hpp"



namespace muffin {


    microSD* microSD::GetInstance()
    {
        return &mInstance;
    }



}