/**
 * @file SpecificAssetId.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * A specific asset ID describes a generic supplementary identifying attribute of the asset.
 * The specific asset ID is not necessarily globally unique.
 * 
 * @note
 * This class inherits from the @class 'HasSemantics'.
 *
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>

#include "./HasSemantics.hpp"
#include "./Reference.hpp"



namespace muffin { namespace aas {


    class SpecificAssetId : public HasSemantics
    {
    private:
        std::string name;
        std::string value;
        Reference mExternalSubjectId;
    };
}}