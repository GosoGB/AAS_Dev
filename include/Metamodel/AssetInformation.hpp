/**
 * @file AssetInformation.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @note 
 * The cardinality of attribute 'defaultThumbnail' is limited to 0 due to memory usage.
 * Hence, the 'defaultThumbnail' is not included in this class.
 * 
 * @todo Need to decide the cardinality of @var 'specificAssetId'
 * 
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <vector>

#include "./HasSemantics.hpp"
#include "./Reference.hpp"
#include "./SpecificAssetId.hpp"
#include "./TypeDefinitions.hpp"
// 



namespace muffin { namespace aas {


    class AssetInformation
    {
    public:
        AssetInformation() {}
        ~AssetInformation() {}
    private:
        asset_kind_e assetKind;
        std::weak_ptr<Reference> globalAssetId;
        std::vector<SpecificAssetId> specificAssetId;
    };
}}