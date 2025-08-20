/**
 * @file IEC63278.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-08-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./Container/include/AASXLoader.hpp"
#include "./Container/Container.hpp"



namespace muffin { namespace aas {


    class IEC63278
    {
    public:
        IEC63278() = default;
        ~IEC63278() noexcept = default;
    public:
        void InitEnvironment()
        {
            Container* container = Container::GetInstance();
            if (container->CountAAS() > 0)
            {
                log_i("Environment already initialized");
                return;
            }
            
            AASXLoader aasxLoader;
            aasxLoader.Start();
        }

        void InitServer()
        {
            ;
        }

        void InitClient()
        {
            ;
        }
    };
}}