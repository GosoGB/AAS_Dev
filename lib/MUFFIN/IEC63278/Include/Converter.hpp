/**
 * @file Converter.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-13
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string.h>

#include <esp32-hal-log.h>

#include "../Metamodel/TypeDefinitions.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    /**
     * @brief Converts a string representation of a key type to its corresponding enum value.
     */
    key_types_e ConvertToKeyType(const char* keyType);


    psram::string ConvertToString(const key_types_e keyType);
    psram::string ConvertToString(const reference_types_e refType);
    psram::string ConvertToString(const asset_kind_e assetKind);
}}