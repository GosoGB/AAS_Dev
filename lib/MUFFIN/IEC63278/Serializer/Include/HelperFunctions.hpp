/**
 * @file HelperFunctions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>

#include "../../Include/Converter.hpp"
#include "../../Metamodel/Extension.hpp"
#include "../../Metamodel/Reference.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    JsonDocument ConvertToJSON(const ExtensionBase& extensionBase);
    JsonDocument ConvertToJSON(const Reference& reference);
    
    psram::string SerializeExtension(const ExtensionBase& extensionBase);
    psram::string SerializeReference(const Reference& reference);

    /**
     * @todo Need to implement deserialization function for Extension class
     */
    // psram::unique_ptr<ExtensionBase> DeserializeExtension(const JsonObject extension);
    Reference DeserializeReference(const JsonObject reference);
}}