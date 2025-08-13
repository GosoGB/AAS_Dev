/**
 * @file AssetAdministrationShellDeserializer.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "ArduinoJson.h"

#include "../../Metamodel/AssetInformation.hpp"
#include "../../Metamodel/AssetAdministrationShell.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class AssetAdministrationShellDeserializer
    {
    public:
        AssetAdministrationShellDeserializer() = default;
        ~AssetAdministrationShellDeserializer() noexcept = default;

    public:
        /**
         * @todo Need to implement attribute 'derivedFrom'
         */
        psram::unique_ptr<AssetAdministrationShell> Parse(const JsonObject payload);

    private:
        /**
         * @todo Need to implement attribute 'specificAssetId'
         */
        AssetInformation parseAssetInformation(const JsonObject payload);

        /**
         * @note 
         * I.A.W. JSON format of AAS, JSON property representing an aggregation attribute must be 
         * omitted if the aggregation is empty. Hence, the size of the property must be greater than 0.
         */
        psram::vector<Reference> parseReferenceToSubmodels(const JsonArray payload);
    };
}}