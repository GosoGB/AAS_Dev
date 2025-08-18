/**
 * @file SubmodelsDeserializer.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "ArduinoJson.h"

#include "../../Metamodel/Abstract/Submodel/Submodel.hpp"
#include "../../Metamodel/Abstract/Submodel/Property.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class SubmodelsDeserializer
    {
    public:
        SubmodelsDeserializer() = default;
        ~SubmodelsDeserializer() noexcept = default;
    public:
        psram::unique_ptr<Submodel> Parse(const JsonObject payload);
    private:
        psram::unique_ptr<DataElement> parseProperty(const JsonObject payload);
    };
}}