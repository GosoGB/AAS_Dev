/**
 * @file AssetAdministrationShellSerializer.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>

#include "../../Metamodel/AssetAdministrationShell.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class AssetAdministrationShellSerializer
    {
    public:
        AssetAdministrationShellSerializer() = default;
        ~AssetAdministrationShellSerializer() noexcept = default;

    public:
        psram::string Encode(const Identifier& id);
        psram::string EncodeAll();
    
    private:
        JsonDocument encode(const AssetAdministrationShell& aas);
    };
}}