/**
 * @file HelperFunctions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <ArduinoJson.h>

#include "../../Include/Converter.hpp"
#include "../../Metamodel/Abstract/Submodel/Property.hpp"
#include "../../Metamodel/Abstract/Submodel/SubmodelElement.hpp"
#include "../../Metamodel/Abstract/Submodel/SubmodelElementCollection.hpp"
#include "../../Metamodel/Extension.hpp"
#include "../../Metamodel/Qualifier.hpp"
#include "../../Metamodel/Reference.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    JsonDocument SerializeExtension(const ExtensionBase& extensionBase);
    JsonDocument SerializeQualifier(const QualifierBase& qualifierBase);
    JsonDocument SerializeReference(const Reference& reference);
    JsonDocument SerializeProperty(const DataElement& dataElement);
    JsonDocument SerializeSMC(const SubmodelElementCollection& smc);
    JsonDocument SerializeSubmodelElement(const SubmodelElement& submodelElement);

    /**
     * @todo Need to implement deserialization function for Extension class
     */
    psram::unique_ptr<ExtensionBase> DeserializeExtensions(const JsonArray extensions);
    psram::unique_ptr<QualifierBase> DeserializeQualifiers(const JsonArray qualifiers);
    Reference DeserializeReference(const JsonObject reference);
    psram::unique_ptr<DataElement> DeserializeProperty(const JsonObject payload);
    psram::unique_ptr<SubmodelElementCollection> DeserializeSMC(const JsonObject payload);
    psram::unique_ptr<SubmodelElement> DeserializeSubmodelElement(const JsonObject payload);
}}