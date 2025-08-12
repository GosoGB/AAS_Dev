/**
 * @file IEC63278.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-08-11
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once



namespace muffin { namespace aas {


    class IEC63278
    {
    public:
        IEC63278() = default;
        ~IEC63278() noexcept = default;
    public:
        void InitEnvironment();
        void InitServer();
        void InitClient();
    };
}}