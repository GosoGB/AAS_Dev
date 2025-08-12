/**
 * @file AASXLoader.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>
#include <cstdint>

#include "../../Metamodel/AssetAdministrationShell.hpp"



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
        key_types_e convertToKeyType(const char* strType);
    private:
        psram::vector<Reference> parseSubmodels(const JsonArray& arraySubmodel);
        AssetInformation parseAssetInformation(JsonObject assetInfo);
        void loadAssetAdministrationShells(JsonObject shell);
        // void loadAssetAdministrationShells();
    private:
        uint8_t mNumAASX = 0;
    };
}}