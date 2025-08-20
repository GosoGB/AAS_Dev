/**
 * @file AssetAdministrationShell.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * An Asset Administration Shell.
 * 
 * @details
 * An Administration Shell is uniquely identifiable since it inherits from Identifiable. The 
 * derivedFrom attribute is used to establish a relationship between two Asset Administration 
 * Shells that are derived from each other.
 * 
 * @note
 * The cardinality of attribute 'dataSpecification' is limited to 0..1 due to memory usage.
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <vector>

#include "./Abstract/HasDataSpecification.hpp"
#include "./Abstract/Identifiable.hpp"
#include "./AssetInformation.hpp"
#include "./Reference.hpp"

#include "Common/DataStructure/bitset.h"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class AssetAdministrationShell : public Identifiable, public HasDataSpecification
    {
    public:
        AssetAdministrationShell(const psram::string& identifier,
                                 const AssetInformation& assetInformation)
            : Identifiable(identifier)
            , mAssetInformation(assetInformation)
        {}

        AssetAdministrationShell(const AssetAdministrationShell& other) = delete;
        AssetAdministrationShell(AssetAdministrationShell&& other) noexcept = default;
        
        AssetAdministrationShell& operator=(const AssetAdministrationShell& other) = delete;
        AssetAdministrationShell& operator=(AssetAdministrationShell&& other) noexcept = default;

        ~AssetAdministrationShell() noexcept override = default;

    public:
        void SetDerivedFrom(const Reference& derivedFrom)
        {
            mDerivedFrom = psram::make_unique<Reference>(derivedFrom);
        }

        void SetDerivedFrom(psram::unique_ptr<Reference> derivedFrom)
        {
            mDerivedFrom = std::move(derivedFrom);
        }

        void SetSubmodel(const psram::vector<Reference>& submodel)
        {
            mSubmodel = submodel;
        }

        void SetSubmodel(psram::vector<Reference>&& submodel)
        {
            mSubmodel = std::move(submodel);
        }

    public:
        const Reference* GetDerivedFromOrNull() const noexcept
        {
            return mDerivedFrom.get();
        }

        const AssetInformation& GetAssetInformation() const noexcept
        {
            return mAssetInformation;
        }

        const psram::vector<Reference>& GetSubmodel() const noexcept
        {
            return mSubmodel;
        }

    public:
        AssetAdministrationShell Clone() const
        {
            AssetAdministrationShell clone(mID, mAssetInformation);
            clone.mSubmodel.reserve(mSubmodel.size());
            
            if (this->GetIdShortOrNull() != nullptr)
            {
                clone.SetIdShort(this->GetIdShortOrNull());
            }
            
            if (this->GetCategoryOrNull() != nullptr)
            {
                clone.SetCategory(this->GetCategoryOrNull());
            }

            if (this->GetExtensionOrNull() != nullptr)
            {
                clone.SetExtension(psram::make_unique<ExtensionBase>(*this->GetExtensionOrNull()));
            }

            for (const auto& ref : mSubmodel)
            {
                clone.mSubmodel.emplace_back(ref);
            }

            if (mDerivedFrom)
            {
                clone.mDerivedFrom = psram::make_unique<Reference>(*mDerivedFrom);
            }

            return clone;
        }

    private:
        psram::unique_ptr<Reference> mDerivedFrom;
        AssetInformation mAssetInformation;
        psram::vector<Reference> mSubmodel;
    };
}}