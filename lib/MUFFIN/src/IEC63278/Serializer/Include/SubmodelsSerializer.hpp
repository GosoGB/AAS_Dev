/**
 * @file SubmodelsSerializer.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>

#include "../../Metamodel/Abstract/Submodel/Submodel.hpp"
#include "../../Metamodel/Abstract/Submodel/DataElement.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {

    
    class SubmodelsSerializer
    {
    public:
        SubmodelsSerializer() = default;
        ~SubmodelsSerializer() noexcept = default;
    public:
        psram::string Encode(const Identifier& id);
        psram::string EncodeProperty(const Identifier& id, const psram::string& idShort);
        psram::string EncodeAll();
    private:
        JsonDocument encode(const Submodel& submodel);
    };
}}