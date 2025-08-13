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

#include "../../Metamodel/AssetAdministrationShell.hpp"



namespace muffin { namespace aas {


    class AssetAdministrationShellSerializer
    {
    public:
        AssetAdministrationShellSerializer() = default;
        ~AssetAdministrationShellSerializer() noexcept = default;

    public:
        void Encode() const;
    };
}}