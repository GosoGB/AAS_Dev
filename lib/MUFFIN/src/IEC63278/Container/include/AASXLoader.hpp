/**
 * @file AASXLoader.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>



namespace muffin { namespace aas {


    class AASXLoader
    {
    public:
        AASXLoader() = default;
        ~AASXLoader() noexcept = default;
    
    public:
        void Start();
    private:
        void countAASX();
        void readAASX();
    private:
        void loadAssetAdministrationShells();
        void loadSubmodels();
    private:
        JsonDocument mJsonDocument;
    };
}}