/**
 * @file AssetInformation.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * In AssetInformation identifying meta data of the asset that is represented by an AAS is defined.
 * 
 * @details
 * In AssetInformation identifying meta data of the asset that is represented by an AAS is defined.
 * The asset may either represent an asset type or an asset instance. 
 * 
 * The asset has a globally unique identifier plus – if needed – additional domain specific 
 * (proprietary) identifiers. However, to support the corner case of very first phase of lifecycle 
 * where a stabilised/constant global asset identifier does not already exist, the corresponding 
 * attribute “globalAssetID” is optional.
 * 
 * @note 
 * The cardinality of attribute 'defaultThumbnail' is limited to 0 due to memory usage.
 * Hence, the 'defaultThumbnail' is not included in this class.
 * 
 * @note 
 * The cardinality of attribute 'specificAssetID' is limited to 0..1 due to memory usage.
 * 
 * @date 2025-07-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>

#include "./Reference.hpp"
#include "./SpecificAssetId.hpp"
#include "./TypeDefinitions.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"


namespace muffin { namespace aas {


    class AssetInformation
    {
    public:
        AssetInformation() : mAssetKind(asset_kind_e::INSTANCE) {}
        AssetInformation(const asset_kind_e assetKind) : mAssetKind(assetKind) {}
        AssetInformation(AssetInformation&& other) noexcept = default;
        
        AssetInformation(const AssetInformation& other)
            : mAssetKind(other.mAssetKind)
            , mGlobalAssetID(other.mGlobalAssetID ? psram::make_unique<Reference>(*other.mGlobalAssetID) : nullptr)
            , mSpecificAssetID(other.mSpecificAssetID ? 
                psram::make_unique<SpecificAssetId>(*other.mSpecificAssetID) : nullptr)
        {}

        AssetInformation& operator=(const AssetInformation& other)
        {
            if (this != &other)
            {
                mAssetKind = other.mAssetKind;

                mGlobalAssetID = other.mGlobalAssetID ? 
                    psram::make_unique<Reference>(*other.mGlobalAssetID) : 
                    nullptr;

                mSpecificAssetID = other.mSpecificAssetID ?
                    psram::make_unique<SpecificAssetId>(*other.mSpecificAssetID) : 
                    nullptr;
            }
            return *this;
        }

        AssetInformation& operator=(AssetInformation&& other) noexcept = default;

        ~AssetInformation() = default;

    public:
        void SetAssetKind(const asset_kind_e assetKind)
        {
            mAssetKind = assetKind;
        }

        void SetGlobalAssetID(const psram::string& globalAssetID)
        {
            psram::vector<Key> keys { {key_types_e::GLOBAL_REFERENCE, globalAssetID} };
            mGlobalAssetID = psram::make_unique<Reference>(reference_types_e::GlobalReference, std::move(keys));
        }

        void SetGlobalAssetID(const Reference& globalAssetID)
        {
            mGlobalAssetID = psram::make_unique<Reference>(globalAssetID);
        }

        void SetGlobalAssetID(psram::unique_ptr<Reference> globalAssetID)
        {
            mGlobalAssetID = std::move(globalAssetID);
        }

        void SetSpecificAssetID(const SpecificAssetId& specificAssetID)
        {
            mSpecificAssetID = psram::make_unique<SpecificAssetId>(specificAssetID);
        }

        void SetSpecificAssetID(psram::unique_ptr<SpecificAssetId> specificAssetID)
        {
            mSpecificAssetID = std::move(specificAssetID);
        }

    public:
        asset_kind_e GetAssetKind() const noexcept
        {
            return mAssetKind;
        }

        const Reference* GetGlobalAssetIdOrNULL() const noexcept
        {
            return mGlobalAssetID.get();
        }

        const SpecificAssetId* GetSpecificAssetIdOrNULL() const noexcept
        {
            return mSpecificAssetID.get();
        }

    private:
        asset_kind_e mAssetKind;
        psram::unique_ptr<Reference> mGlobalAssetID;
        psram::unique_ptr<SpecificAssetId> mSpecificAssetID;
    };
}}